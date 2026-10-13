<!-- Author: kelexine <https://github.com/kelexine> -->

# pwroff_alarm

Vendor helper that arms the PMIC power-on alarm for the Cubot P50 (`marlon`,
MT6765/MT6357) so an alarm can wake the phone from full power-off.

## How it fits together

```
framework app --sets--> vendor.pwroff_alarm.epoch --init.target.rc--> pwroff_alarm sync
                                                                        |
                                                  /dev/alarm  ioctl ANDROID_ALARM_SET(POWER_ON)
                                                                        |
                          kernel rtc-mt6397 -> PMIC PDN/SPAR regs -> stock LK (alarm boot, mode 7)
```

- Type 6 (`POWER_ON`, no logo) is deliberate: stock LK only selects ALARM_BOOT
  (boot mode 7) when the logo flag is clear. The timerfd path always sets it.
- The alarm is one-shot: LK consumes it, so the framework must re-arm after
  every alarm boot (it republishes the next alarm at boot).

## Usage

```
pwroff_alarm [--device PATH] set <epoch> | clear | sync
```

| Command      | Effect                                                              |
|--------------|---------------------------------------------------------------------|
| `set <e>`    | Arm at UTC epoch `<e>`; `0`, past or within 5 s of now clears       |
| `clear`      | Disarm                                                              |
| `sync`       | Apply `vendor.pwroff_alarm.epoch`; unset property is a no-op        |

`sync` re-reads the property after applying it (max 5 rounds), so overlapping
helpers started by rapid writes converge on the newest value.

Input is validated before the device is opened: digits only, <= 2095-12-31
(the kernel rejects `tm_year > 195`). A bad write never disturbs an armed alarm.

Exit codes: `0` ok, `64` usage, `65` bad/out-of-range epoch, `74` device error.

## Known limits (not fixed here)

- LK accepts the wake only within `[T-1s, T+4s]` of the stored time; boot
  latency leaves little margin. Measure it before relying on it.
- Ordinary suspend alarms rewrite the RTC alarm register; the driver only
  re-arms it at shutdown in mode 5. Needs a kernel-side fix if it bites.

## Tests

```
tests/run_host_tests.sh          # needs clang++ or g++ and libgtest-dev
```

Runs the gtest suites (`tests/unit`, `tests/integration`) and a smoke test of
the real binary (`tests/cli_smoke.sh`). Under Soong: `pwroff_alarm_tests`.
