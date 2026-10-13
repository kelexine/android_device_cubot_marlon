#!/usr/bin/env python3
# Author: kelexine <https://github.com/kelexine>
# SPDX-License-Identifier: Apache-2.0
"""Mutation tests: the contract checker must pass on the real tree and fail,
with a pointed message, when each cross-file promise is broken."""
import pathlib
import shutil
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import check_init_contracts as cic  # noqa: E402

REAL_ROOT = pathlib.Path(__file__).resolve().parents[2]
RC = "rootdir/etc/init.target.rc"


class ContractTests(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self._tmp.name) / "dt"
        shutil.copytree(REAL_ROOT, self.root, ignore=shutil.ignore_patterns(
            ".git", "__pycache__", "*.o", "*.pyc"))

    def tearDown(self):
        self._tmp.cleanup()

    def mutate(self, rel, old, new):
        path = self.root / rel
        text = path.read_text()
        self.assertIn(old, text, f"mutation anchor missing in {rel}")
        path.write_text(text.replace(old, new, 1))

    def assertCaught(self, fragment):
        errors = cic.check_all(self.root)
        self.assertTrue(any(fragment in e for e in errors),
                        f"expected an error containing {fragment!r}, got {errors}")

    def test_real_tree_is_clean(self):
        self.assertEqual(cic.check_all(self.root), [])

    # --- rc syntax -------------------------------------------------------
    def test_tab_indent_is_rejected(self):
        self.mutate(RC, "    exec_background", "\texec_background")
        self.assertCaught("tab character")

    def test_wrong_indent_is_rejected(self):
        self.mutate(RC, "    exec_background", "  exec_background")
        self.assertCaught("indent must be exactly 4 spaces")

    def test_exec_needs_separator(self):
        self.mutate(RC, "system system -- /vendor", "system system /vendor")
        self.assertCaught("exec without ' -- ' separator")

    def test_command_outside_section_is_rejected(self):
        self.mutate(RC, "on property:vendor.pwroff_alarm.epoch=*\n",
                    "    stray_command\non property:vendor.pwroff_alarm.epoch=*\n")
        self.assertCaught("command outside any section")

    # --- alarm contract --------------------------------------------------
    def test_property_name_drift_between_rc_and_helper(self):
        self.mutate(RC, "vendor.pwroff_alarm.epoch=*", "vendor.pwroff_alarm.epoc=*")
        self.assertCaught("matching kEpochProperty")

    def test_property_not_labelled(self):
        self.mutate("sepolicy/vendor/property_contexts",
                    "vendor.pwroff_alarm.", "vendor.other_prefix.")
        self.assertCaught("nothing labels vendor.pwroff_alarm.epoch")

    def test_property_type_must_be_vendor_public(self):
        self.mutate("sepolicy/vendor/pwroff_alarm.te",
                    "vendor_public_prop(vendor_pwroff_alarm_prop)", "")
        self.assertCaught("not declared vendor_public_prop")

    def test_binary_missing_from_soong(self):
        self.mutate("pwroff_alarm/Android.bp", 'name: "pwroff_alarm",\n    defaults',
                    'name: "pwroff_alarm_x",\n    defaults')
        self.assertCaught("no cc_binary named pwroff_alarm")

    def test_binary_must_be_vendor(self):
        self.mutate("pwroff_alarm/Android.bp", "    vendor: true,\n    srcs: [\"src/main.cpp\"]",
                    "    srcs: [\"src/main.cpp\"]")
        self.assertCaught("is not vendor: true")

    def test_binary_missing_from_product_packages(self):
        self.mutate("device.mk", "    pwroff_alarm \\\n", "")
        self.assertCaught("pwroff_alarm missing from PRODUCT_PACKAGES")

    def test_binary_without_file_label(self):
        self.mutate("sepolicy/vendor/file_contexts",
                    "/vendor/bin/pwroff_alarm    u:object_r:pwroff_alarm_exec:s0", "")
        self.assertCaught("/vendor/bin/pwroff_alarm has no label")

    def test_exec_type_must_be_declared(self):
        self.mutate("sepolicy/vendor/pwroff_alarm.te",
                    "type pwroff_alarm_exec, exec_type", "type pwroff_alarm_exec_x, exec_type")
        self.assertCaught("exec type pwroff_alarm_exec is not declared")

    # --- services --------------------------------------------------------
    def test_service_seclabel_type_must_exist(self):
        self.mutate(RC, "seclabel u:r:earlylog:s0", "seclabel u:r:earlylogx:s0")
        self.assertCaught("seclabel type earlylogx is not declared")

    def test_service_requires_seclabel_user_group(self):
        self.mutate(RC, "    seclabel u:r:earlylog:s0\n", "")
        self.assertCaught("missing 'seclabel'")
        self.mutate(RC, "    user logd\n", "")
        self.assertCaught("missing 'user'")

    # --- logging ---------------------------------------------------------
    def test_log_dir_must_be_created(self):
        self.mutate(RC, "mkdir /metadata/earlylog 0770", "mkdir /metadata/earlylogs 0770")
        self.assertCaught("is not created by any 'mkdir'")

    def test_log_dir_must_be_labelled(self):
        self.mutate("sepolicy/system_ext/private/file_contexts",
                    "/metadata/earlylog(/.*)?", "/metadata/somethingelse(/.*)?")
        self.assertCaught("/metadata/earlylog has no label")

    def test_log_data_type_must_be_declared(self):
        self.mutate("sepolicy/system_ext/private/earlylog.te",
                    "type earlylog_data_file, file_type;", "")
        self.assertCaught("data type earlylog_data_file is not declared")

    def test_log_cap_is_enforced(self):
        self.mutate(RC, "-r 1024 -n 4", "-r 1024 -n 40")
        self.assertCaught("exceeds budget")

    def test_cap_counts_the_live_file(self):
        # (n + 1) files: n=5 at 1024 KiB is exactly the 6144 KiB budget, n=6 is over.
        self.mutate(RC, "-r 1024 -n 4", "-r 1024 -n 5")
        self.assertEqual(cic.check_logging_contract(self.root), [])
        self.mutate(RC, "-r 1024 -n 5", "-r 1024 -n 6")
        self.assertCaught("exceeds budget")

    def test_kernel_buffer_needs_logd_property(self):
        self.mutate("system_ext.prop", "ro.logd.kernel=true", "")
        self.assertCaught("ro.logd.kernel=true required")

    def test_earlylog_must_start_on_init(self):
        self.mutate(RC, "    start earlylog\n", "")
        self.assertCaught("nothing starts earlylog on init")

    def test_earlylog_must_be_disabled(self):
        self.mutate(RC, "    disabled\n", "")
        self.assertCaught("must be 'disabled'")

    # --- install wiring --------------------------------------------------
    def test_rc_must_install_to_auto_parsed_dir(self):
        self.mutate("rootdir/Android.bp", 'src: "etc/init.target.rc",\n    sub_dir: "init",',
                    'src: "etc/init.target.rc",\n    sub_dir: "init/hw",')
        self.assertCaught('must use sub_dir "init"')

    def test_rc_missing_from_product_packages(self):
        self.mutate("device.mk", "    init.target.rc \\\n", "")
        self.assertCaught("init.target.rc missing from PRODUCT_PACKAGES")

    def test_board_config_must_include_system_ext_policy(self):
        self.mutate("BoardConfig.mk", "SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS +=",
                    "SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS :=")
        self.assertCaught("SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS does not include")


if __name__ == "__main__":
    unittest.main(verbosity=1)
