# Adhan C++

Adhan C++ is a C++20 port of [adhan-js](https://github.com/batoulapps/adhan-js) 💜 for calculating Islamic prayer times.

All of the astronomical logic, the calculation methods, and the overall semantics of this library come directly from the original TypeScript implementation. This port only translates that logic into C++; the underlying math and design were already worked out by the adhan-js authors and contributors. If you are looking for the reference implementation, or want to understand the astronomical formulas in more depth, that repository is the right place to start.

## Table of Contents

- [Requirements](#requirements)
- [Building the library](#building-the-library)
- [Usage](#usage)
  - [Coordinates](#coordinates)
  - [Date](#date)
  - [Calculation parameters](#calculation-parameters)
  - [Prayer times](#prayer-times)
  - [Convenience utilities](#convenience-utilities)
  - [Sunnah times](#sunnah-times)
  - [Qibla direction](#qibla-direction)
- [Running the tests](#running-the-tests)
- [Examples](#examples)
  - [CLI](#cli)
  - [Browser parity check](#browser-parity-check)
- [Installing](#installing)
- [License](#license)

## Requirements

- CMake 4.3 or newer
- A C++20 compiler (GCC 13+ or a comparable Clang build with full C++20 `<chrono>` calendar support)
- No third party runtime dependencies. The library itself has no external dependencies; a couple of test-only dependencies are described below in the [tests](#running-the-tests) section.

`JSDate`'s local time handling relies on `<chrono>`'s time zone database support, which in turn depends on the platform having a usable IANA time zone database (`tzdata`) available. Some platforms and toolchains do not ship this, for example Termux on Android at the time of writing.

There is no automatic detection of this at configure time. If your platform lacks it, the regular build will fail to compile with errors pointing at `zoned_time`, `current_zone`, or similar. When that happens, configure with the `ADHAN_USE_CTIME_FALLBACK` option instead, which compiles the library against a `localtime_r`/`mktime` based implementation instead of `<chrono>`'s calendar and time zone support:

```bash
cmake -S . -B build -DADHAN_USE_CTIME_FALLBACK=ON
```

See the [Date](#date) and [Running the tests](#running-the-tests) sections for what this affects.

## Building the library

The project uses CMake, with an out of source build directory.

```bash
cmake -S . -B build
cmake --build build
```

This configures and builds `libadhan` (a shared library by default) into the `build` directory, along with the test suite (built by default, see [Running the tests](#running-the-tests)).

Useful options, passed with `-D` at the configure step:

- `-DADHAN_USE_CTIME_FALLBACK=ON` — use the `localtime_r`/`mktime` fallback described in [Requirements](#requirements). Default is `OFF`.
- `-DBUILD_TESTS=OFF` — skip building the test suite. Default is `ON`.
- `-DBUILD_EXAMPLES=ON` — build the examples under `examples/`, including `adhan-cli`. Default is `OFF`.
- `-DCMAKE_BUILD_TYPE=Release` — an optimized build (`-O2`, `NDEBUG` defined, so internal `assert()` checks such as the one in `JSDate::getTime()` are compiled out). If `CMAKE_BUILD_TYPE` is left unset, CMake does not add any optimization flags by default, so it is worth setting explicitly.
- `-DCMAKE_BUILD_TYPE=Debug` — unoptimized, with debug symbols and asserts left active.

These can be combined, for example:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DADHAN_USE_CTIME_FALLBACK=ON -DBUILD_EXAMPLES=ON
cmake --build build
```

To reconfigure with different options later, either pass the new `-D` flags to the same `cmake -S . -B build ...` command again, or remove the `build` directory and configure from scratch if switching options like `ADHAN_USE_CTIME_FALLBACK` produces unexpected results.

## Usage

Headers live under `include/adhan`, and everything is scoped under the `adhan` namespace.

```cpp
#include <adhan/Coordinates.hpp>
#include <adhan/CalculationMethod.hpp>
#include <adhan/PrayerTimes.hpp>
#include <adhan/JSDate.hpp>

int main() {
  adhan::Coordinates coordinates(35.789751, -78.691249);
  adhan::CalculationParameters params = adhan::CalculationMethod::NorthAmerica();
  adhan::PrayerTimes prayerTimes(coordinates, adhan::JSDate::now(), params);

  // prayerTimes.fajr, .sunrise, .dhuhr, .asr, .maghrib, .isha
  // are each a JSDate representing that prayer time in UTC.
}
```

### Coordinates

Create a `Coordinates` object with the latitude and longitude for the location you want prayer times for.

```cpp
adhan::Coordinates coordinates(35.78056, -78.6389);
```

### Date

`JSDate` is this library's date type. It mirrors the parts of JavaScript's `Date` that the original library actually relies on: constructing a date from year, month, and day (month is zero indexed, matching JavaScript), reading back local or UTC components, and getting the current time.

```cpp
adhan::JSDate date = adhan::JSDate::now();        // current date and time
adhan::JSDate specific(2026, 0, 1);               // January 1, 2026
```

Only the year, month, and day matter for prayer time calculation; any time of day components are ignored for that purpose. Internally, `JSDate` reads local time through the platform's `<chrono>` time zone database, so the system needs a usable time zone database available for local time components to resolve correctly.

On platforms where that database is not available, configure with `-DADHAN_USE_CTIME_FALLBACK=ON` as described in [Requirements](#requirements). This compiles `JSDate` against `localtime_r`/`mktime` instead. Local time components still work correctly there, since they defer to the operating system's own time zone handling, but this is worth knowing if you are digging into `JSDate`'s implementation and see two code paths gated behind `ADHAN_USE_CTIME_FALLBACK`.

> **IMPORTANT**:
> <br>
> This is NOT a full-fledged port of JS's `Date`! Only the "required" parts were ported as needed. If you need date parsing and other calculations done on your C++ codebase, I highly recommend using a more feature-rich date library, like Howard Hinnant's [well-regarded library](https://github.com/HowardHinnant/date).
> <br>--- OR ---<br>
> Use a framework-included one like Qt's [QDate](https://doc.qt.io/qt-6/qdate.html) or Boost's [DateTime](https://www.boost.org/library/latest/date_time/)

### Calculation parameters

Prayer time calculation needs a `CalculationParameters` instance, which you typically get from a preset method and then adjust as needed.

```cpp
adhan::CalculationParameters params = adhan::CalculationMethod::MoonsightingCommittee();
params.madhab = adhan::Madhab::Hanafi;
params.highLatitudeRule = adhan::HighLatitudeRule::TwilightAngle;
```

See `CalculationMethod.hpp` for the full list of available presets (MuslimWorldLeague, Egyptian, Karachi, UmmAlQura, Dubai, MoonsightingCommittee, NorthAmerica, Kuwait, Qatar, Singapore, Turkey, Tehran, and Other).

### Prayer times

```cpp
adhan::PrayerTimes prayerTimes(coordinates, date, params);
```

The resulting object exposes `fajr`, `sunrise`, `dhuhr`, `asr`, `sunset`, `maghrib`, and `isha`, each a `JSDate` in UTC.

### Convenience utilities

```cpp
adhan::Prayer current = prayerTimes.currentPrayer();
adhan::Prayer next = prayerTimes.nextPrayer();
auto nextTime = prayerTimes.timeForPrayer(next); // std::optional<adhan::JSDate>
```

### Sunnah times

```cpp
#include <adhan/SunnahTimes.hpp>

adhan::SunnahTimes sunnahTimes(prayerTimes);

// sunnahTimes.middleOfTheNight
// sunnahTimes.lastThirdOfTheNight
```

### Qibla direction

Get the direction, in degrees from North, of the Qibla from a given set of coordinates.

```cpp
#include <adhan/Qibla.hpp>

double direction = adhan::qibla(coordinates);
```

## Running the tests

Tests use [doctest](https://github.com/doctest/doctest) and live under `tests/`. The test executable is built from the library's own sources directly (not linked against `libadhan`), compiled with `ADHAN_TESTING` defined, since a few tests (such as the polar circle resolver call counting) rely on test-only instrumentation that only exists under that definition. This keeps the shipped library itself free of any test-only code. A couple of the fixture tests also use [nlohmann/json](https://github.com/nlohmann/json), vendored under `tests/vendor`.

Tests are built by default (`BUILD_TESTS` is `ON`). After building:

```bash
cmake --build build --target test
```

or, to also print successful assertions:

```bash
cmake --build build --target test-verbose
```

Both write their output to a log file in the project root (`test.log` or `test-verbose.log`) in addition to the terminal. See `tests/` for the individual test files, which mirror the upstream adhan-js test suite one file at a time so behavior can be checked against the original implementation.

A handful of test cases rely on formatting prayer times in an arbitrary named time zone (see `formatInZone` in `tests/src`), so they can compare against the fixed expected values in the upstream test suite. This formatting is test-only tooling; it is not part of the shipped library, and it needs the same `<chrono>` time zone database described in [Requirements](#requirements) to work.

On an `ADHAN_USE_CTIME_FALLBACK` build, this test-only formatting has no way to resolve an arbitrary named zone, so those specific test cases are excluded rather than run incorrectly. This means the fallback build has slightly less test coverage than a build with full time zone database support, though it does not affect the fallback build's correctness for prayer time calculation itself. There is currently no plan to pull in an extra date library dependency just to cover this gap on incompatible systems.

## Examples

Although there are several examples in this repository under `examples`, they are mostly for sanity checks during development. With the exception of `examples/adhan-cli`, its okay to overlook these.

The key example worth talking about is `examples/adhan-cli`, which provides a look at the library usage in a real example, while also letting us check output parity between the original library (see [browser parity check](#browser-parity-check) below for more details).

Examples are not built by default. Configure with `-DBUILD_EXAMPLES=ON` to include them:

```bash
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build
```

### CLI

`examples/adhan-cli` is a small command line program that exercises the public API end to end: prayer times, Sunnah times, Qibla direction, and a couple of convenience utilities.

```bash
./build/examples/adhan-cli/adhan-cli 23.775787 90.368047 2026-07-24 MuslimWorldLeague Shafi MiddleOfTheNight General Up
```

Run it with no arguments to see the full list of accepted options.

### Browser parity check

`examples/adhan-cli/parity-check-with-js` is a small HTML page used to cross check the C++ CLI's output against the original adhan-js library in a browser, using the same inputs. It loads `adhan.umd.min.js` as a plain script tag, so it can be opened directly in a browser without needing a local server.

## Installing

```bash
cmake --install build --prefix /desired/install/path
```

This installs `libadhan` (archive and/or shared library, depending on platform and whether it was built shared) and the headers under `include/adhan`, so they can be consumed from another project without needing this repository's build tree directly.

## License

MIT, matching the license of the upstream [adhan-js](https://github.com/batoulapps/adhan-js) project. See `LICENSE`.
