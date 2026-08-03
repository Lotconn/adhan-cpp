# Adhan C++

Adhan C++ is a C++20 port of [adhan-js](https://github.com/batoulapps/adhan-js) 💜
for calculating Islamic prayer times.

The astronomical logic, the calculation methods, and the semantics of this library
come from the original TypeScript implementation.
This port translates that logic into C++. The authors and contributors of adhan-js
did the work on the math and the design.
You are encouraged to visit the adhan-js repository for the reference implementation
or to learn more about the astronomical formulas.

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

- You must use CMake 4.3 or a later version.

- You must use a C++20 compiler.
  Use GCC 13 or a later version.
  Or use a Clang build that has full C++20 `<chrono>` calendar support.

- The library has no dependency on other software at run time.
  The [tests](#running-the-tests) section describes two dependencies that
  apply only to the tests.

`DateTime` reads local time through the `<chrono>` time zone database.
This function needs an IANA time zone database (`tzdata`) on the platform.
Some platforms and toolchains do not have this database.
One example is Termux on Android.

The build process does not check for this database at configure time.
If the platform does not have the database, the normal build will not compile.
The build will show errors about `zoned_time`, `current_zone`, or similar items.
If this occurs, configure the build with the `ADHAN_USE_CTIME_FALLBACK` option.
This option compiles the library with a `localtime_r`/`mktime` based function.
This function replaces the `<chrono>` calendar and time zone function.

```bash
cmake -S . -B build -DADHAN_USE_CTIME_FALLBACK=ON
```

Go to the [Date](#date) section and the [Running the tests](#running-the-tests)
section for more data about this option.

## Building the library

This project uses CMake. You are free to use an out-of-source build directory,
or do it in-source. The documentation here will use the former.

```bash
cmake -S . -B build
cmake --build build
```

This procedure configures and builds `libadhan` in the `build` directory.
By default, `libadhan` is a shared library. This also builds the test suite by default.
Check [Running the tests](#running-the-tests) for more info.

You can add these options at the configure step, with the `-D` flag:

## CMake Options

- `-DADHAN_USE_CTIME_FALLBACK=ON` (default: `OFF`)
  Use the localtime_r/mktime fallback function from the Requirements
  section.

- `-DBUILD_TESTS=OFF` (default: `ON`)
  Do not build the test suite.

- `-DBUILD_EXAMPLES=ON` (default: `OFF`)
  Build the examples in the examples directory. This includes
  adhan-cli.

- `-DCMAKE_BUILD_TYPE=Release`
  Use this option for an optimized build. This option uses the `-O2`
  flag and defines `NDEBUG`. This disables internal `assert()` checks,
  for example the check in `DateTime::getTime()`. If you do not set
  `CMAKE_BUILD_TYPE`, CMake does not add optimization flags. Its
  recommended to set this option.

- `-DCMAKE_BUILD_TYPE=Debug`
  Use this option for a build with no optimization. This option keeps
  debug symbols and `assert()` checks active.

You can combine these options, for example:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DADHAN_USE_CTIME_FALLBACK=ON -DBUILD_EXAMPLES=ON
cmake --build build
```

To use different options later, do either one of the following:

- Run the same `cmake -S . -B build <...>` command again, with the new `-D` flags.
- Remove the `build` directory. Then configure the build again. Use this procedure
  if an option like `ADHAN_USE_CTIME_FALLBACK` gives unexpected results.

## Usage

The headers are in the `include/adhan` directory. All items are in the `Adhan` namespace.

```cpp
#include <adhan/Coordinates.hpp>
#include <adhan/CalculationMethod.hpp>
#include <adhan/PrayerTimes.hpp>
#include <adhan/DateTime.hpp>

int main() {
  Adhan::Coordinates coordinates(35.789751, -78.691249);
  Adhan::CalculationParameters params = Adhan::CalculationMethod::NorthAmerica();
  Adhan::PrayerTimes prayerTimes(coordinates, Adhan::DateTime::now(), params);

  // prayerTimes.fajr, .sunrise, .dhuhr, .asr, .maghrib, .isha
  // are each a DateTime representing that prayer time in UTC.
}
```

### Coordinates

Create a `Coordinates` object. Use the latitude and the longitude of the location.

```cpp
Adhan::Coordinates coordinates(35.78056, -78.6389);
```

### Date

`DateTime` is the date type of this library. `DateTime` gives you the parts of the
JavaScript `Date` type that this library needs. You can construct a date from a year,
a month, and a day. The month uses the same zero-based numbers as JavaScript.
You can read local values or UTC values from a `DateTime` object.
It is possible to also get the current date and time.

```cpp
Adhan::DateTime specific(2026, 0, 1);               // January 1, 2026
Adhan::DateTime date = adhan::DateTime::now();      // current date and time
```

The prayer time calculation uses only the year, the month, and the day.
The calculation ignores the time of day. Internally, `DateTime` reads local time
through the `<chrono>` [time zone database](https://en.wikipedia.org/wiki/Tz_database).
Because of this, the system needs a usable time zone database.
This lets local time values give correct results.

### The Timezone Database Fallback

If the platform does not have the database mentioned in the section above, configure
the build with `-DADHAN_USE_CTIME_FALLBACK=ON`. See the [Requirements](#requirements)
section. This option compiles `DateTime` with `localtime_r`/`mktime` functions instead.

Local time values still give correct results with this option. These functions use
the time zone data from the operating system. This is useful data if you read the
code of `DateTime` and see two sets of functions. These two sets of functions use
the `ADHAN_USE_CTIME_FALLBACK` definition to select between them.

This option is not set as the default in order to adhere to modern C++ standards.

> **IMPORTANT**:
>
> This is not a full port of the JavaScript `Date` class! This port has only the
> parts that this library needs. If your C++ code needs more date functions, for
> example date parsing, use a library that has more datetime utilities. One example
> is the highly regarded [date library](https://github.com/HowardHinnant/date)
> from Howard Hinnant.
>
> --- OR ---
>
> Use a date library from your framework of choice. Two examples are Qt's
> [QDate](https://doc.qt.io/qt-6/qdate.html)
> and Boost's [DateTime](https://www.boost.org/library/latest/date_time/).

### Calculation parameters

The prayer time calculation needs a `CalculationParameters` object.
Get an instance of this object from one of the preset methods.
Then change the values that you need, like this:

```cpp
Adhan::CalculationParameters params = Adhan::CalculationMethod::MoonsightingCommittee();
params.madhab = Adhan::Madhab::Hanafi;
params.highLatitudeRule = Adhan::HighLatitudeRule::TwilightAngle;
```

Go to `CalculationMethod.hpp` for the full list of preset methods, which include:

`MuslimWorldLeague`, `Egyptian`, `Karachi`, `UmmAlQura`, `Dubai`, `MoonsightingCommittee`,
`NorthAmerica`, `Kuwait`, `Qatar`, `Singapore`, `Turkey`, `Tehran`, and `Other`.

### Prayer times

```cpp
Adhan::PrayerTimes prayerTimes(coordinates, date, params);
```

This object gives you these values: `fajr`, `sunrise`, `dhuhr`, `asr`, `sunset`,
`maghrib`, and `isha`. Each value is a `DateTime` object in UTC. You can refer
to the `examples/adhan-cli` for information on how to use this properly.

### Convenience utilities

```cpp
Adhan::Prayer current = prayerTimes.currentPrayer();
Adhan::Prayer next = prayerTimes.nextPrayer();
auto nextTime = prayerTimes.timeForPrayer(next); // std::optional<Adhan::DateTime>
```

### Sunnah times

```cpp
#include <adhan/SunnahTimes.hpp>

Adhan::SunnahTimes sunnahTimes(prayerTimes);

// sunnahTimes.middleOfTheNight
// sunnahTimes.lastThirdOfTheNight
```

### Qibla direction

Use this function to get the Qibla direction.
The function gives the direction in degrees from North.
Use a set of coordinates as the input.

```cpp
#include <adhan/Qibla.hpp>

double direction = Adhan::qibla(coordinates);
```

## Running the tests

The tests use [doctest](https://github.com/doctest/doctest).
The tests are in the `tests` directory.
The build process compiles the test executable from the source files of the library.
The test executable does not link to `libadhan`.
The build process compiles the tests with the `ADHAN_TESTING` definition.
Some tests need this definition, for example
the tests that count calls to the polar circle resolver.
This procedure keeps the shipped library free of test code.
Some fixture tests also use [nlohmann/json](https://github.com/nlohmann/json).
This library is in the `tests/vendor` directory.

The build process builds the tests by default. The `BUILD_TESTS` option has the
default value `ON`. After you build the project, run one of these commands.

```bash
cmake --build build --target test
```

Use this command to also print the assertions that pass.

```bash
cmake --build build --target test-verbose
```

Both commands write output to a log file in the project root directory.
The files are `test.log` and `test-verbose.log`. Both commands also print
output to the terminal. Go to the `tests` directory for the test files.
Each file corresponds to one file in the adhan-js test suite. Use these files
to check the behavior of this port against the original implementation.

Some tests need to format prayer times in a named time zone. See `formatInZone`
in the `tests/src` directory. These tests compare the result against fixed
values from the adhan-js test suite. This function is only for tests.
The function is not part of the shipped library. The function needs the same
`<chrono>` time zone database from the [Requirements](#requirements) section.

If you build with the `ADHAN_USE_CTIME_FALLBACK` option, this test function cannot
find a named time zone. Because of this, the build process removes these specific
tests. The build process does not run these tests with wrong results.
Because of this, a fallback build has fewer tests than a build with full time
zone database support. This does not change the correctness of the fallback build
for prayer time calculation.

At this time, we do not plan to add another date library just to close this gap only.

## Examples

This repository has several examples in the `examples` directory.
Most examples are only for checks during development.
You can ignore most of these examples.
The example `examples/adhan-cli` is a bit different.

The `examples/adhan-cli` example shows library usage in a real stateless program.
This example also lets you check the output against the original library.
See the [browser parity check](#browser-parity-check) section for more data.

The build process does not build the examples by default.
Configure the build with `-DBUILD_EXAMPLES=ON` to build the examples.

```bash
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build
```

### CLI

`examples/adhan-cli` is a small command line program. This program uses the full
public interface of the library. The program calculates prayer times, Sunnah times,
and the Qibla direction. The program also shows some convenience functions.

<!-- markdownlint-disable MD013 -->
```bash
./build/examples/adhan-cli/adhan-cli 23.775787 90.368047 2026-07-24 MuslimWorldLeague Shafi MiddleOfTheNight General Up
```
<!-- markdownlint-enable MD013 -->

Run the program with no arguments to see the full list of options.

### Browser parity check

`examples/adhan-cli/parity-check-with-js` is a small HTML page. It was made to check
the output of the C++ CLI program against the original `adhan-js` library, in a browser,
after being given the same input values for both programs.

This page loads `adhan.umd.min.js` as a normal script tag. Because of this, you can
open the page directly in a browser without the need for a local server.

## Installing

```bash
cmake --install build --prefix /desired/install/path
```

This command installs `libadhan`. This is an archive file or a shared library file,
based on your platform and your build options. This command also installs the headers
from the `include/adhan` directory.
After this command, other projects can use this library.
These projects do not need the build directory of this repository.

## License

This project uses the MIT license.
This is the same license as the upstream [adhan-js](https://github.com/batoulapps/adhan-js)
project. See the `LICENSE` file.
