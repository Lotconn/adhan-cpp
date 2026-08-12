# Fixture Conversion

The shared prayer time fixtures in `tests/Shared/Times` come from
[adhan-testdata](https://github.com/batoulapps/adhan-testdata) and record
their times as local wall clock readings, with the zone named in
`params.timezone`.

The library hands back UTC instants and never reads a time zone database.
Rather than make the tests convert at run time, which would drag a tzdb
back in, these scripts do the conversion once and write the result to
`tests/Shared/Times/UTC`. That is what lets the suite build and run on
libc++, on macOS, and anywhere else the tzdb is missing or incomplete.

Both scripts need Python 3.10 or newer for `zoneinfo`, and nothing else.

## generate_utc_fixtures.py

Reads every fixture in `tests/Shared/Times` and writes a converted twin
named `<name>_UTC.json` into the `UTC` subdirectory. Files left behind by an
earlier naming scheme are deleted, because the tests read every json file in
that directory and a stale one would be counted as an extra fixture.

```bash
python3 tools/fixtures/generate_utc_fixtures.py
python3 tools/fixtures/generate_utc_fixtures.py --check
python3 tools/fixtures/generate_utc_fixtures.py --convert "Europe/Oslo" "2020-06-15 01:14"
```

`--check` reports whether the converted files are current. `--convert` turns
one local reading into UTC and prints a ready made `CHECK(isUtc(...))` line,
which is what you want when adding a test case by hand.

## verify_utc_fixtures.py

Re-derives every value straight from the original fixtures and compares.

This exists because `--check` re-runs the generator's own conversion code on
both sides, so it catches a stale file but would happily reproduce a wrong
conversion. This script works the answers out independently, so a bug in the
generator shows up as a mismatch.

```bash
python3 tools/fixtures/verify_utc_fixtures.py
```

Prefer this one in CI. It is the stronger check, and it does not compare the
recorded `tzdataVersion`, so a tzdata package upgrade will not report the
fixtures stale when every time value is unchanged.

## When to run these

Rarely. The upstream fixtures have not been touched since June 2021 and
every date in them is in the past, so the converted files are effectively
frozen. Run the generator if you sync new fixtures from upstream, and the
verifier once afterwards.
