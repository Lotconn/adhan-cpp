# Adhan C++

Adhan C++ is a C++20 port of [adhan-js](https://github.com/batoulapps/adhan-js) for calculating Islamic prayer times.

All of the astronomical logic, the calculation methods, and the overall semantics of this library come directly from the original TypeScript implementation. This port only translates that logic into C++; the underlying math and design were already worked out by the adhan-js authors and contributors. If you are looking for the reference implementation, or want to understand the astronomical formulas in more depth, that repository is the right place to start.

## Requirements

- A C++20 compiler (GCC 13+ or a comparable Clang build with full C++20 `<chrono>` calendar support)
- No third party runtime dependencies. The library itself has no external dependencies; a couple of test-only dependencies are described below.

## Building the library

The project builds as a static library using the provided Makefile.

```bash
make
```

This produces `libadhan.a` in the project root, along with object files under `obj/`.

```bash
make clean
```

removes the build output.

```bash
make help
```

show a list of available commands.

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

Tests use [doctest](https://github.com/doctest/doctest) and live under `tests/`, kept entirely separate from the shipped library so the test framework never ends up in `libadhan.a`. A couple of the fixture tests also use [nlohmann/json](https://github.com/nlohmann/json), vendored under `tests/vendor`.

```bash
make test
```

This compiles the test sources with the library sources and runs the resulting binary. See `tests/` for the individual test files, which mirror the upstream adhan-js test suite one file at a time so behavior can be checked against the original implementation.

## Examples

### CLI

`examples/adhan-cli` is a small command line program that exercises the public API end to end: prayer times, Sunnah times, Qibla direction, and a couple of convenience utilities.

```bash
cd examples/adhan-cli
make
./bin/adhan-cli 23.775787 90.368047 2026-07-24 MuslimWorldLeague Shafi MiddleOfTheNight General Up
```

Run it with no arguments to see the full list of accepted options.

### Browser parity check

`examples/adhan-cli/parity-check-with-js` is a small HTML page used to cross check the C++ CLI's output against the original adhan-js library in a browser, using the same inputs. It loads `adhan.umd.min.js` as a plain script tag, so it can be opened directly in a browser without needing a local server.

## License

MIT, matching the license of the upstream adhan-js project. See `LICENSE`.
