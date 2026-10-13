#!/usr/bin/env python3
# Author: kelexine <https://github.com/kelexine>
# SPDX-License-Identifier: Apache-2.0
"""Static contract checks for init.target.rc and what it depends on.

There is no AOSP tree in CI, so nothing compiles the rc/sepolicy/Soong glue
together. This checks the cross-file promises that otherwise only break at
boot: property names, SELinux types, binary labels, install wiring and the
/metadata log size cap. Usage: check_init_contracts.py [device-tree-root]
"""
import pathlib
import re
import sys

# Total rolling-log budget inside the 16 MiB /metadata (it also holds OTA and
# GSI state, so stay well under half).
MAX_LOG_KIB = 6144

RC_REL = "rootdir/etc/init.target.rc"
EPOCH_HEADER_REL = "pwroff_alarm/include/pwroff_alarm/cli.h"


def _read(root, rel):
    return (root / rel).read_text()


def parse_rc(text):
    """Returns (sections, errors); a section is {'kind','name','header','lines'}."""
    sections, errors, current = [], [], None
    for no, raw in enumerate(text.splitlines(), 1):
        if "\t" in raw:
            errors.append(f"rc:{no}: tab character (init rc uses 4-space indent)")
        if raw != raw.rstrip():
            errors.append(f"rc:{no}: trailing whitespace")
        stripped = raw.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if not raw.startswith(" "):
            m = re.match(r"(on|service)\s+(\S+)", stripped)
            if not m:
                errors.append(f"rc:{no}: unindented line is not a section: {stripped!r}")
                current = None
                continue
            current = {"kind": m.group(1), "name": m.group(2), "header": stripped, "lines": []}
            sections.append(current)
        else:
            if current is None:
                errors.append(f"rc:{no}: command outside any section: {stripped!r}")
                continue
            if not raw.startswith("    ") or raw.startswith("     "):
                errors.append(f"rc:{no}: indent must be exactly 4 spaces")
            if re.match(r"exec(_background|_start)?\s", stripped) and " -- " not in stripped:
                errors.append(f"rc:{no}: exec without ' -- ' separator: {stripped!r}")
            current["lines"].append(stripped)
    names = [s["name"] for s in sections if s["kind"] == "service"]
    for dup in {n for n in names if names.count(n) > 1}:
        errors.append(f"rc: duplicate service {dup!r}")
    return sections, errors


def declared_types(root):
    types = set()
    for te in (root / "sepolicy").rglob("*.te"):
        text = te.read_text()
        types.update(re.findall(r"^\s*type\s+(\w+)\s*[,;]", text, re.M))
        types.update(re.findall(r"vendor_public_prop\(\s*(\w+)\s*\)", text))
    return types


def prop_types_declared_public(root):
    out = set()
    for te in (root / "sepolicy").rglob("*.te"):
        out.update(re.findall(r"vendor_public_prop\(\s*(\w+)\s*\)", te.read_text()))
    return out


def context_entries(root, rels):
    """[(pattern, type)] from file_contexts / property_contexts style files."""
    entries = []
    for rel in rels:
        path = root / rel
        if not path.exists():
            continue
        for line in path.read_text().splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            m = re.match(r"(\S+)\s+(?:--\s+|-d\s+)?u:object_r:(\w+):s0", line)
            if m:
                entries.append((m.group(1), m.group(2)))
    return entries


def soong_blocks(root, module_type):
    """[(name, body)] for every `module_type { ... }` in every Android.bp."""
    found = []
    for bp in root.rglob("Android.bp"):
        text = bp.read_text()
        for m in re.finditer(rf"^{module_type}\s*\{{", text, re.M):
            depth, i = 1, m.end()
            while i < len(text) and depth:
                depth += {"{": 1, "}": -1}.get(text[i], 0)
                i += 1
            body = text[m.end():i - 1]
            name = re.search(r'name:\s*"([^"]+)"', body)
            if name:
                found.append((name.group(1), body))
    return found


def product_packages(root):
    text = _read(root, "device.mk")
    packages = set()
    for m in re.finditer(r"PRODUCT_PACKAGES\s*\+=\s*((?:[^\n]*\\\n)*[^\n]*)", text):
        packages.update(re.findall(r"[\w.@+-]+", m.group(1).replace("\\", " ")))
    return packages


def check_rc_syntax(root):
    _, errors = parse_rc(_read(root, RC_REL))
    return errors


def check_alarm_contract(root):
    errors = []
    sections, _ = parse_rc(_read(root, RC_REL))
    header = _read(root, EPOCH_HEADER_REL)
    m = re.search(r'kEpochProperty\s*=\s*"([^"]+)"', header)
    if not m:
        return ["cli.h: kEpochProperty not found"]
    prop = m.group(1)

    triggers = [s["name"] for s in sections if s["kind"] == "on"
                and s["name"].startswith("property:")]
    if f"property:{prop}=*" not in triggers:
        errors.append(f"rc: no 'on property:{prop}=*' trigger matching kEpochProperty")

    prop_ctx = context_entries(root, ["sepolicy/vendor/property_contexts"])
    owners = [t for pattern, t in prop_ctx if prop.startswith(pattern)]
    if not owners:
        errors.append(f"property_contexts: nothing labels {prop}")
    for t in owners:
        if t not in prop_types_declared_public(root):
            errors.append(f"sepolicy: property type {t} is not declared vendor_public_prop")

    for section in sections:
        for line in section["lines"]:
            m = re.search(r"(/vendor/bin/(\w+))", line)
            if m and line.startswith("exec"):
                errors += _check_vendor_binary(root, m.group(1), m.group(2))
    return errors


def _check_vendor_binary(root, path, name):
    errors = []
    bins = {n: b for n, b in soong_blocks(root, "cc_binary")}
    if name not in bins:
        errors.append(f"Soong: no cc_binary named {name} for {path}")
    elif not re.search(r"vendor:\s*true", bins[name]):
        errors.append(f"Soong: cc_binary {name} is not vendor: true")
    if name not in product_packages(root):
        errors.append(f"device.mk: {name} missing from PRODUCT_PACKAGES")
    labels = [t for pattern, t in context_entries(root, ["sepolicy/vendor/file_contexts"])
              if pattern == path]
    if not labels:
        errors.append(f"file_contexts: {path} has no label")
    for t in labels:
        if t not in declared_types(root):
            errors.append(f"sepolicy: exec type {t} is not declared")
    return errors


def check_services(root):
    errors = []
    sections, _ = parse_rc(_read(root, RC_REL))
    types = declared_types(root)
    for s in sections:
        if s["kind"] != "service":
            continue
        opts = s["lines"]
        if not any(o.startswith("user ") for o in opts):
            errors.append(f"service {s['name']}: missing 'user'")
        if not any(o.startswith("group ") for o in opts):
            errors.append(f"service {s['name']}: missing 'group'")
        label = next((o for o in opts if o.startswith("seclabel ")), None)
        if label is None:
            errors.append(f"service {s['name']}: missing 'seclabel'")
            continue
        m = re.match(r"seclabel u:r:(\w+):s0$", label)
        if not m:
            errors.append(f"service {s['name']}: malformed {label!r}")
        elif m.group(1) not in types:
            errors.append(f"service {s['name']}: seclabel type {m.group(1)} is not declared")
    return errors


def check_logging_contract(root):
    errors = []
    sections, _ = parse_rc(_read(root, RC_REL))
    svc = next((s for s in sections if s["kind"] == "service" and s["name"] == "earlylog"), None)
    if svc is None:
        return ["rc: no earlylog service"]
    command = svc["header"]
    f = re.search(r"-f\s+(/\S+)/[^/\s]+", command)
    r = re.search(r"-r\s+(\d+)", command)
    n = re.search(r"-n\s+(\d+)", command)
    if not (f and r and n):
        return ["rc: earlylog needs -f <dir>/<name>, -r <KiB> and -n <count>"]
    log_dir = f.group(1)

    mkdirs = [line.split()[1] for s in sections for line in s["lines"] if line.startswith("mkdir ")]
    if log_dir not in mkdirs:
        errors.append(f"rc: log dir {log_dir} is not created by any 'mkdir'")

    entries = context_entries(root, ["sepolicy/system_ext/private/file_contexts"])
    owners = [t for pattern, t in entries if re.fullmatch(pattern, log_dir)]
    if not owners:
        errors.append(f"file_contexts: {log_dir} has no label")
    for t in owners:
        if t not in declared_types(root):
            errors.append(f"sepolicy: data type {t} is not declared")

    total = (int(n.group(1)) + 1) * int(r.group(1))  # logcat keeps file + n rotated
    if total > MAX_LOG_KIB:
        errors.append(f"rc: log cap {total} KiB exceeds budget {MAX_LOG_KIB} KiB")

    if "kernel" in command.split("-b", 1)[1].split()[0]:
        prop = (root / "system_ext.prop").read_text()
        if not re.search(r"^ro\.logd\.kernel=true\s*$", prop, re.M):
            errors.append("system_ext.prop: ro.logd.kernel=true required for -b kernel")

    mk_actions = [s for s in sections if s["kind"] == "on" and s["name"] == "init"
                  and any(line == "start earlylog" for line in s["lines"])]
    if not mk_actions:
        errors.append("rc: nothing starts earlylog on init (after logd)")
    if "disabled" not in svc["lines"]:
        errors.append("rc: earlylog must be 'disabled' so only the init action starts it")
    return errors


def check_install_wiring(root):
    errors = []
    etc = dict(soong_blocks(root, "prebuilt_etc"))
    body = etc.get("init.target.rc")
    if body is None:
        errors.append("Soong: no prebuilt_etc named init.target.rc")
    else:
        if 'sub_dir: "init"' not in body:
            errors.append('Soong: init.target.rc must use sub_dir "init" (auto-parsed dir)')
        if not re.search(r"vendor:\s*true", body):
            errors.append("Soong: init.target.rc must be vendor: true")
    if "init.target.rc" not in product_packages(root):
        errors.append("device.mk: init.target.rc missing from PRODUCT_PACKAGES")

    board = _read(root, "BoardConfig.mk")
    for var, rel in (("BOARD_VENDOR_SEPOLICY_DIRS", "sepolicy/vendor"),
                     ("SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS", "sepolicy/system_ext/private")):
        if not re.search(rf"{var}\s*\+=\s*\$\(DEVICE_PATH\)/{re.escape(rel)}\s*$", board, re.M):
            errors.append(f"BoardConfig.mk: {var} does not include {rel}")
        if not (root / rel).is_dir():
            errors.append(f"{rel} does not exist")
    return errors


CHECKS = (check_rc_syntax, check_alarm_contract, check_services,
          check_logging_contract, check_install_wiring)


def check_all(root):
    root = pathlib.Path(root)
    errors = []
    for check in CHECKS:
        errors += [f"[{check.__name__}] {e}" for e in check(root)]
    return errors


def main(argv):
    root = pathlib.Path(argv[1]) if len(argv) > 1 else pathlib.Path(__file__).resolve().parents[2]
    errors = check_all(root)
    for check in CHECKS:
        bad = [e for e in errors if e.startswith(f"[{check.__name__}]")]
        print(("FAIL " if bad else "ok   ") + check.__name__)
    for e in errors:
        print("  - " + e)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
