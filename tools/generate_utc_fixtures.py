#!/usr/bin/env python3
"""Turn the shared prayer time fixtures into UTC ones.

The fixtures under tests/Shared/Times come from batoulapps/adhan-testdata and
record prayer times as local wall clock readings, with an IANA zone name in
the params block. Reading those at test time means the test binary needs a
time zone database, which is exactly what the library was changed to avoid.

So the conversion happens here instead, once, and the result goes to
tests/Shared/Times/UTC. Every time string becomes an explicit UTC date and
time, the timezone field is dropped, and the tests can then compare instants
without looking anything up.

This is safe to freeze because nothing involved moves. The upstream data has
not been touched since June 2021 and the newest fixture date is 2020-12-31,
so the offsets are settled history. The tzdata version used is written into
each generated file for the record.

Usage:
    python3 tools/generate_utc_fixtures.py            # write the UTC fixtures
    python3 tools/generate_utc_fixtures.py --check    # verify they are current
    python3 tools/generate_utc_fixtures.py --convert "Europe/Oslo" \\
        "2020-06-15 01:14"                            # one off, for new tests

The --convert mode is there for when someone adds a test case and needs the
UTC instant that goes with a published local time.
"""

from __future__ import annotations

import argparse
import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from zoneinfo import ZoneInfo



def tzdata_version() -> str:
    """Best effort read of which IANA release the conversion used."""
    try:
        from tzdata import IANA_VERSION
        return IANA_VERSION
    except ImportError:
        pass

    marker = Path("/usr/share/zoneinfo/+VERSION")
    if marker.is_file():
        return marker.read_text().strip()

    # Debian and Ubuntu drop the +VERSION marker, so ask the package manager
    try:
        import subprocess
        out = subprocess.run(
            ["dpkg-query", "-W", "-f=${Version}", "tzdata"],
            capture_output=True, text=True, check=True).stdout.strip()
        if out:
            return out.split("-")[0]
    except Exception:
        pass

    return "unknown"


REPO_ROOT = Path(__file__).resolve().parent.parent
SOURCE_DIR = REPO_ROOT / "tests" / "Shared" / "Times"
OUTPUT_DIR = SOURCE_DIR / "UTC"

TIME_KEYS = ("fajr", "sunrise", "dhuhr", "asr", "maghrib", "isha")

def utc_name(source: Path) -> Path:
    """Where a source fixture's converted twin lives, e.g. Doha-Qatar_UTC.json."""
    return OUTPUT_DIR / (source.stem + "_UTC.json")


def parse_local(date_str: str, time_str: str, zone: ZoneInfo) -> datetime:
    """Read a fixture date and a "5:44 AM" style time as a local datetime."""
    hour_minute, meridiem = time_str.split(" ")
    hour, minute = (int(part) for part in hour_minute.split(":"))

    hour %= 12
    if meridiem.upper() == "PM":
        hour += 12

    year, month, day = (int(part) for part in date_str.split("-"))
    return datetime(year, month, day, hour, minute, tzinfo=zone)


def convert_file(path: Path) -> dict:
    """Build the UTC version of one fixture."""
    data = json.loads(path.read_text())
    params = dict(data["params"])
    zone_name = params.pop("timezone")
    zone = ZoneInfo(zone_name)

    times = []
    for entry in data["times"]:
        converted = {"date": entry["date"]}
        for key in TIME_KEYS:
            local = parse_local(entry["date"], entry[key], zone)
            utc = local.astimezone(timezone.utc)
            converted[key] = utc.strftime("%Y-%m-%d %H:%M")
        times.append(converted)

    result = {
        "generatedBy": "tools/generate_utc_fixtures.py",
        "source": path.name,
        "sourceTimezone": zone_name,
        "tzdataVersion": tzdata_version(),
        "params": params,
        "times": times,
    }
    if "variance" in data:
        result["variance"] = data["variance"]
    return result


def render(payload: dict) -> str:
    return json.dumps(payload, indent=2) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check", action="store_true",
        help="exit non-zero if the generated fixtures are missing or stale")
    parser.add_argument(
        "--convert", nargs=2, metavar=("ZONE", "LOCAL"),
        help='convert one local reading, e.g. "Europe/Oslo" "2020-06-15 01:14"')
    args = parser.parse_args()

    if args.convert:
        zone_name, local_str = args.convert
        local = datetime.strptime(local_str, "%Y-%m-%d %H:%M").replace(
            tzinfo=ZoneInfo(zone_name))
        utc = local.astimezone(timezone.utc)
        print("local : %s %s" % (local.strftime("%Y-%m-%d %H:%M"), zone_name))
        print("UTC   : %sZ" % utc.strftime("%Y-%m-%d %H:%M"))
        print("check : CHECK(isUtc(x, %dy/%s/%dd, %dh + %dmin));" % (
            utc.year, utc.strftime("%B"), utc.day, utc.hour, utc.minute))
        return 0

    sources = sorted(SOURCE_DIR.glob("*.json"))
    if not sources:
        print("No fixtures found in %s" % SOURCE_DIR, file=sys.stderr)
        return 1

    if args.check:
        stale = []
        for source in sources:
            target = OUTPUT_DIR / source.name
            if not target.exists() or target.read_text() != render(
                    convert_file(source)):
                stale.append(source.name)
        if stale:
            print("Out of date, rerun this script without --check:",
                  file=sys.stderr)
            for name in stale:
                print("  %s" % name, file=sys.stderr)
            return 1
        print("%d UTC fixtures are current" % len(sources))
        return 0

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    # Drop anything left over from an earlier naming scheme. The tests read
    # every json file in this directory, so a stale one would be counted as
    # an extra fixture rather than ignored.

    expected = {utc_name(source) for source in sources}

    for leftover in sorted(set(OUTPUT_DIR.glob("*.json")) - expected):
        leftover.unlink()
        print("%-40s removed (stale)" % leftover.name)

    for source in sources:
        target = utc_name(source)
        target.write_text(render(convert_file(source)))
        print("%-40s -> %s" % (source.name, target.relative_to(REPO_ROOT)))

    print("\n%d fixtures written using tzdata %s" % (len(sources),
                                                     tzdata_version()))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
