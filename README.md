# Adhan C++

[![CI](https://github.com/lotconn/adhan-cpp/actions/workflows/ci.yml/badge.svg?branch=v1.0.0)](https://github.com/lotconn/adhan-cpp/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/lotconn/adhan-cpp)](LICENSE)
[![Release](https://img.shields.io/github/v/release/lotconn/adhan-cpp)](https://github.com/lotconn/adhan-cpp/releases)

Adhan C++ is a C++20 port of [adhan-js](https://github.com/batoulapps/adhan-js) 💜
for calculating Islamic prayer times.

The astronomical logic, the calculation methods, and the semantics of this library
come from the original TypeScript implementation.
This port translates that logic into C++. The authors and contributors of adhan-js
did the work on the math and the design.
You are encouraged to visit the adhan-js repository for the reference implementation
or to learn more about the astronomical formulas.

This Adhan C++ library is made to be minimal, cross-platform, and dependency-free.

## Table of Contents

- [Requirements](#requirements)
- [Building the library](#building-the-library)
- [CMake Options](#cmake-options)
- [Usage](#usage)
  - [Coordinates](#coordinates)
  - [Date](#date)
  - [Time zones](#time-zones)
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
- [Importing into another project](#importing-into-another-project)
- [License](#license)

## Requirements

- You must use CMake 3.21 or a later version.

- You must use a C++20 compiler with `<chrono>` calendar support.
  `Suggested:` GCC 13 or later, or Clang 17 or later.
  GCC 12 builds the library itself, but not the tests or the examples,
  which stream `<chrono>` types to an ostream.

- The library has no dependency on other software at run time.
  The [tests](#running-the-tests) section describes one dependency that
  applies only to the tests.

- The library does **not** need an IANA time zone database (`tzdata`).
  It takes a plain calendar date and returns instants in UTC.
  This matters on platforms where the `<chrono>` time zone database is missing
  or incomplete, such as libc++ before version 19, macOS, and Termux on Android.
  The library builds and runs the same way on all of them.

  Read the [Time zones](#time-zones) section for the reasoning and for what to
  do when you want a local wall clock reading.

## Building the library

This project uses CMake. You are free to use an out-of-source build directory,
or do it in-source. The documentation here will use the former.

```bash
cmake -S . -B build
cmake --build build --parallel
```

This procedure configures and builds `libadhan` in the `build` directory.
By default, `libadhan` is a shared library. This also builds the test suite by default.

Check [Running the tests](#running-the-tests) for more info.

You can add these options at the configure step, with the `-D` flag.
See [available options](#cmake-options).

## CMake Options

- `-DBUILD_SHARED_LIBS=OFF` (default: `ON`)
  Build `libadhan` as a static library instead of a shared library.

- `-DADHAN_BUILD_TESTS=OFF` (default: `ON`)
  Do not build the test suite.

- `-DADHAN_BUILD_EXAMPLES=ON` (default: `OFF`)
  Build the examples in the examples directory. This includes
  adhan-cli.

- `-DADHAN_ENABLE_CLANG_TIDY=OFF` (default: `ON` when this project is the
  top level one, `OFF` when it is pulled in by another project)
  Run clang-tidy on the library sources during the build. If the option is
  on and clang-tidy is not found, the build carries on and prints a warning.

- `-DCMAKE_BUILD_TYPE=Release`
  Use this option for an optimized build. This option uses the `-O2`
  flag and defines `NDEBUG`.

- `-DCMAKE_BUILD_TYPE=Debug`
  Use this option for a build with no optimization. This option keeps
  debug symbols and `assert()` checks active.

> Note: `Release` is the default option for this project's `CMAKE_BUILD_TYPE`

You can combine these options, for example:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DADHAN_BUILD_EXAMPLES=ON
cmake --build build --parallel
```

To use different options later, do either one of the following:

- Run the same `cmake -S . -B build <...>` command again, with the new `-D` flags.
- Remove the `build` directory. Then configure the build again. Use this procedure
  if an option gives unexpected results.

## Usage

The headers are in the `include/adhan` directory. All items are in the `Adhan` namespace.

```cpp
#include <adhan/CalculationMethod.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateUtils.hpp>
#include <adhan/PrayerTimes.hpp>

#include <chrono>

int main() {
  using namespace std::chrono;

  Adhan::Coordinates coordinates(35.789751, -78.691249);
  Adhan::CalculationParameters params = Adhan::CalculationMethod::NorthAmerica();
  year_month_day date{2026y / January / 1d};

  Adhan::PrayerTimes prayerTimes(coordinates, date, params);

  // prayerTimes.fajr, .sunrise, .dhuhr, .asr, .sunset, .maghrib, .isha
  // are each an Adhan::OptInstant holding that prayer time in UTC.
}
```

### Coordinates

Create a `Coordinates` object. Use the latitude and the longitude of the location.

```cpp
Adhan::Coordinates coordinates(35.78056, -78.6389);
```

### Date

The library uses `<chrono>` types directly. There is no date class of its own.

The date you pass in is a `std::chrono::year_month_day`. It is a calendar date
and nothing else. It carries no time of day and belongs to no time zone.
Months are named or one-based, not zero-based like the JavaScript original.

```cpp
using namespace std::chrono;

year_month_day date{2026y / January / 1d};
```

The times you get back are instants in UTC. Two aliases in `DateUtils.hpp`
name them:

```cpp
using Instant = std::chrono::sys_time<std::chrono::milliseconds>;
using OptInstant = std::optional<Instant>;
```

`Instant` is at millisecond resolution because a JavaScript `Date` is, so the
ported arithmetic rounds off in the same place the original does.

`OptInstant` is an `Instant` that may be absent. The original returns an
Invalid Date when a time cannot be worked out, which mostly happens near the
poles. An empty `std::optional` says the same thing, and the compiler makes
you handle it.

> **NOTE**
>
> Take care when comparing an `OptInstant`. `std::optional` treats an empty
> one as smaller than any value, whereas any comparison against an Invalid
> Date in JavaScript is false. Check `has_value()` first and compare the
> contents, rather than comparing two optionals directly.

For today's date in UTC:

```cpp
auto now = Adhan::now();
auto today = std::chrono::year_month_day{
              std::chrono::floor<std::chrono::days>(now)
            };
```

Cheatsheet:

```cpp
/* UTC midnight that begins the day now falls in */
const auto todayStart = std::chrono::floor<std::chrono::days>(now);

/* That same day as a plain calendar date, no time attached */
const auto today = std::chrono::year_month_day{todayStart};

/* How far past that midnight now is, as whole hours, minutes and seconds */
const auto time = std::chrono::hh_mm_ss{
    std::chrono::floor<std::chrono::seconds>(now - todayStart)};

const int year = int(today.year()); // Note the signed int for `year`
const unsigned month = unsigned(today.month());
const unsigned day = unsigned(today.day());

const auto hour = time.hours().count();
const auto minute = time.minutes().count();
const auto second = time.seconds().count();
```

### Time zones

The library never reads a time zone database. That is deliberate, and it is
why the library builds on platforms where `<chrono>`'s tzdb is unavailable.

Only two things ever needed a time zone, and both sit at the edges rather than
in the calculation:

- Deciding which calendar day a person is currently in.
- Showing a result on a local wall clock.

Both are yours to do, at whichever boundary suits your program. If you already
know the date you want, the first one does not come up at all.

```cpp
using namespace std::chrono;

// The calendar day it is right now in a named zone
auto localNow = zoned_time{locate_zone("America/New_York"),
                           system_clock::now()}.get_local_time();
year_month_day today{floor<days>(localNow)};

Adhan::PrayerTimes prayerTimes(coordinates, today, params);

// One of the results, read on that same local wall clock
if (prayerTimes.maghrib) {
  zoned_time shown{locate_zone("America/New_York"), *prayerTimes.maghrib};
  std::cout << shown.get_local_time() << '\n';
}
```

Note that a prayer time for a local day often falls on a different UTC day.
Fajr on 1 January in Singapore is on 31 December in UTC. Both readings name
the same instant.

If your platform has no usable tzdb, you can supply a fixed offset yourself,
read one from your framework, or use the
[date library](https://github.com/HowardHinnant/date) from Howard Hinnant.
None of that touches the library.

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
`maghrib`, and `isha`. Each value is an `Adhan::OptInstant` in UTC. You can refer
to the `examples/adhan-cli` for information on how to use this properly.

```cpp
if (prayerTimes.fajr) {
  Adhan::Instant fajr = *prayerTimes.fajr;
}
```

### Convenience utilities

```cpp
Adhan::Prayer current = prayerTimes.currentPrayer();
Adhan::Prayer next = prayerTimes.nextPrayer();
Adhan::OptInstant nextTime = prayerTimes.timeForPrayer(next);
```

`currentPrayer` and `nextPrayer` take an `Adhan::Instant` and default to the
current time. The answers only mean something when the object holds today's
times. `timeForPrayer` returns an empty optional for `Prayer::None` and for any
prayer whose time could not be worked out.

### Sunnah times

```cpp
#include <adhan/SunnahTimes.hpp>

Adhan::SunnahTimes sunnahTimes(prayerTimes);

// sunnahTimes.middleOfTheNight
// sunnahTimes.lastThirdOfTheNight
```

Both values are an `Adhan::OptInstant`. They are absent when either end of the
night is missing.

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
The test executable links to `libadhan` rather than compiling the library
sources a second time.
Some fixture tests also use [nlohmann/json](https://github.com/nlohmann/json).
This library is in the `tests/vendor` directory.

The build process builds the tests by default. The `ADHAN_BUILD_TESTS` option
has the default value `ON`. After you build the project, run one of these commands.

```bash
ctest --test-dir build --output-on-failure
# which does the same thing as:
cmake --build build --target test
```

To run the test executable itself and see its own output, use the `libtest`
target. Add `-s` to also print the assertions that pass. Example:

```bash
cmake --build build --target libtest --parallel
./build/tests/run-tests -s
```

Go to the `tests` directory for the test files.
Most files correspond to one file in the adhan-js test suite. Use these files
to check the behavior of this port against the original implementation.

The tests need no time zone database either. The expected times come from the
adhan-js test suite, which records them as local wall clock readings. Those
readings are converted to UTC ahead of time, not while the tests run. Fixture
data is converted by the scripts in `tools/fixtures`, and the times written by
hand in the test files are stored as UTC with the local reading kept in a
comment beside them. This means the whole suite builds and runs everywhere the
library does.

## Examples

This repository has several examples in the `examples` directory.
Most examples are only for checks during development.
You can ignore most of these examples.
The example `examples/adhan-cli` is a bit different.

The `examples/adhan-cli` example shows library usage in a real stateless program.
This example also lets you check the output against the original library.
See the [browser parity check](#browser-parity-check) section for more data.

The build process does not build the examples by default.
Configure the build with `-DADHAN_BUILD_EXAMPLES=ON` to build the examples.

```bash
cmake -S . -B build -DADHAN_BUILD_EXAMPLES=ON
cmake --build build --parallel
```

### CLI

`examples/adhan-cli` is a small command line program. This program uses the full
public interface of the library. The program calculates prayer times, Sunnah times,
and the Qibla direction. The program also shows some convenience functions.

The program works in UTC throughout. It takes a UTC calendar day and prints UTC
times, so it needs no time zone database of its own.

<!-- markdownlint-disable MD013 -->

```bash
./build/examples/adhan-cli/adhan-cli --latitude 23.775787 --longitude 90.368047 --date 2026-07-24 --method MuslimWorldLeague --madhab Shafi --high-latitude-rule TwilightAngle --rounding Nearest
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

> Note: On Windows, a `Debug` build of your project needs
> a `Debug` build of this library, and the same applies for `Release`.

## Importing into another project

You can bring this library into another CMake project any way you want.
CMake supports several methods for this, such as `FetchContent`, `find_package`
after an install, git submodules, or a package manager.

The example below uses `FetchContent`. This is only one example method, not the
only supported one. Pick whichever method fits your own project.

```cmake
# Fetch `adhan` in your current CMake project
#
include(FetchContent)
FetchContent_Declare(
  adhan
  GIT_REPOSITORY https://<this-repo-url>  # placeholder used, populate as needed
  GIT_TAG <tag-or-commit>                 # use the latest/desired tag
  GIT_SHALLOW TRUE                        # project has no dependencies
)
FetchContent_MakeAvailable(adhan)
#
# (Or just build + install)

# Link the `adhan` library to your project
#
target_link_libraries(${TARGET_NAME} PUBLIC
  adhan::adhan
)

# Use
#
#include <adhan/Adhan.hpp>

std::cout << "ADHANLIB:\t\t";
std::cout << Adhan::project_version << std::endl;
```

The `GIT_REPOSITORY` and `GIT_TAG` values above are placeholders on purpose.
Put in the actual URL and tag or commit you want yourself, instead of copying
the lines as they are. This also matters if you work from a fork: an unfilled
placeholder stops you from fetching the upstream repository by accident.

## License

This project uses the MIT license.
This is the same license as the upstream [adhan-js](https://github.com/batoulapps/adhan-js)
project. See the `LICENSE` file.
