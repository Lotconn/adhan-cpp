# Build Matrix

> These tools are for inspecting matrix builds locally. Use at your own risk!

This tool validates every supported build configuration for a project and generates
an HTML report you can browse locally. It is designed to be project agnostic.
All settings specific to your CMake project live in
`tools/build_matrix/project.json`. You can reuse this directory for a different
project by editing that file or pointing to a new one with the
`--project-config` flag. Nothing else needs to change.

## Requirements

- Python 3.10 or newer
- CMake
- A supported C++ compiler
- Ninja (optional but strongly recommended for performance)

## Configuring Your Project

Edit `tools/build_matrix/project.json` to match your setup.

```json
{
  "package_name": "adhan",
  "link_target": "adhan::adhan",
  "shared_libs_var": "BUILD_SHARED_LIBS",
  "build_type_var": "CMAKE_BUILD_TYPE",
  "cmake_minimum_version": "3.20",
  "test_target": "test",
  "examples_subdirectory": "examples",
  "tests_option": "build_tests",
  "examples_option": "build_examples",
  "options": [
    { "id": "build_tests", "cmake_var": "ADHAN_BUILD_TESTS" },
    { "id": "build_examples", "cmake_var": "ADHAN_BUILD_EXAMPLES" }
  ]
}
```

The `package_name` and `link_target` fields let the install check find and link
against your installed target. The `options` array defines arbitrary boolean
CMake options. Every combination of these options crossed with library type,
build type, and install status becomes a matrix case.
Feel free to add, remove, or rename entries.

Set `tests_option` and `examples_option` to the option ID that gates building tests
or examples. Use `null` if neither applies. The `test_target` field specifies which
CMake target runs your tests. The `examples_subdirectory` tells the tool where to
find built example executables relative to the build directory.

## Running the Tool

Run every configuration with this command.

```bash
python tools/build_matrix/build_matrix.py
```

You can customize the run with these flags.

- `--source /path/to/project` to specify a source directory
- `--project-config /path/to/project.json` to use a different config file
- `--output build-matrix-report` to choose an output directory
- `--jobs 16` to control parallelism per build
- `--workers 8` to build multiple cases at once (defaults to min(4, cpu count))
- `--only <case-id>` to run a single specific case
- `--resume` to skip cases that already passed or failed and retry aborted ones
- `--verbose` to print every command and dump output for failures

When `--workers` is greater than one and `--jobs` is not set, the tool automatically
splits available CPU cores across workers so builds do not compete for resources.

## What Gets Tested

The matrix covers every combination of shared or static libraries, all standard build
types, every boolean option toggled on and off, and installation enabled or disabled.
With the two options above that comes to 64 cases: 2 library types, 4 build types,
2 settings per option, and 2 install states.

There used to be a third option, `ctime_fallback`, which selected a `<ctime>`
implementation for platforms whose standard library shipped no `tzdb`. The library
no longer reads a time zone database at all, so that option is gone from CMake and
the matrix is half the size it was.

When the configured test option is on, the tool builds and runs the test target.
When examples are enabled, it discovers and runs every executable in the
examples subdirectory. Any negative exit code fails the case.

For configurations with installation enabled, the tool performs the install,
generates a standalone consumer project from templates, builds it against the
install tree, runs the consumer executable, and then cleans up the temporary files.

## A Note on the adhan-cli Help Case

`project.json` expects `adhan-cli --help` to exit with 1 rather than 0. That is
not a mistake in the config. The CLI bails out early when it is given fewer than
three arguments, prints usage, and returns 1, so `--help` on its own never reaches
the branch that would return 0. The case records what the program does today. If
you decide help should exit cleanly, change the CLI and this expectation together.

## Performance Notes

The tool uses Ninja automatically when it is available on your PATH.
This makes a significant difference on Windows where the default Visual Studio
generator is slower and stores output in per-configuration subdirectories that require
extra searching. Installing Ninja via pip or your package manager speeds up builds
and removes that ambiguity. Results remain correct regardless of generator because
the tool always passes the build type explicitly.

Cases build in parallel by default. A failure in one case never stops the
rest of the matrix.
Unexpected errors are captured per case rather than crashing the entire run.

## Output and Reports

The tool creates a `build-matrix-report` directory containing metadata, matrix data,
individual logs, case JSON files, and an HTML report. Every build starts clean and
always includes the parallel flag. Each case produces its own log and JSON summary
so you can inspect partially completed runs.

Open `build-matrix-report/html/index.html` in any modern browser to view the report.
It includes search, pass/fail/error summaries, color-coded case lists with side-by-side
build logs, timing data, artifact lists, executed examples, and complete logs.
You can drag the divider between the case list and log viewer to resize them.
