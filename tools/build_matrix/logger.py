from __future__ import annotations

import subprocess
import threading
import time
from collections.abc import Sequence
from pathlib import Path

from models import CommandResult


class Logger:

    def __init__(self, config):

        self.config = config
        self._print_lock = threading.Lock()

    def info(
        self,
        message: str,
    ) -> None:

        with self._print_lock:
            print(message, flush=True)

    def separator(
        self,
    ) -> None:

        self.info("=" * 80)

    def _write_header(
        self,
        fp,
        title: str,
    ) -> None:

        fp.write("=" * 80)
        fp.write("\n")

        fp.write(title)
        fp.write("\n")

        fp.write("=" * 80)
        fp.write("\n\n")

    def run(
        self,
        command: list[str],
        cwd: Path,
        logfile: Path,
        *,
        case_name: str | None = None,
        step_name: str | None = None,
        expected_returncodes: tuple[int, ...] = (0,),
        expect_stdout_contains: Sequence[str] = (),
        timeout: float | None = None,
    ) -> CommandResult:
        """
        Run `command` and record the result.

        `expected_returncodes` defaults to (0,) so every build-system
        command is checked strictly. Only example programs pass something
        else, driven by the "examples" section of project.json.

        `expect_stdout_contains` adds an output assertion on top of the
        exit code: every substring must appear in stdout or the command
        fails. This is how an example proves it did some work rather than
        merely starting and stopping cleanly.
        """

        logfile.parent.mkdir(
            parents=True,
            exist_ok=True,
        )

        label = " / ".join(
            part for part in (case_name, step_name) if part
        )

        if self.config.verbose:
            prefix = f"[{label}] " if label else ""
            self.info(f"{prefix}$ {' '.join(command)}")

        start = time.perf_counter()

        launch_error: str | None = None

        try:

            process = subprocess.run(
                command,
                cwd=cwd,
                capture_output=True,
                text=True,
                shell=False,
                check=False,
                encoding="utf-8",
                errors="replace",
                timeout=timeout,
            )

            stdout = process.stdout
            stderr = process.stderr
            returncode = process.returncode

        except subprocess.TimeoutExpired as exc:

            stdout = exc.stdout or ""
            stderr = (exc.stderr or "") + f"\nTimed out after {timeout}s."
            returncode = -1
            launch_error = f"TimeoutExpired: timed out after {timeout}s"

        except Exception as exc:

            # The process never started. This is distinct from "the
            # process ran and exited with an unexpected code", and it is
            # never a success regardless of expected_returncodes.
            stdout = ""
            stderr = str(exc)
            returncode = -1
            launch_error = f"{type(exc).__name__}: {exc}"

        elapsed = (
            time.perf_counter()
            - start
        )

        missing_output = [
            text
            for text in expect_stdout_contains
            if text not in stdout
        ]

        result = CommandResult(
            command=command,
            cwd=str(cwd),
            returncode=returncode,
            elapsed=elapsed,
            stdout=stdout,
            stderr=stderr,
            expected_returncodes=tuple(expected_returncodes),
            missing_output=missing_output,
            launch_error=launch_error,
        )

        # Multiple threads never share a logfile (one per case), so no
        # lock is needed for the file write itself.
        with logfile.open(
            "a",
            encoding="utf-8",
        ) as fp:

            self._write_header(
                fp,
                f"COMMAND - {step_name}" if step_name else "COMMAND",
            )

            fp.write(
                " ".join(command)
            )

            fp.write("\n\n")

            fp.write(
                f"Working directory : {cwd}\n"
            )

            fp.write(
                f"Elapsed           : {elapsed:.3f} s\n"
            )

            fp.write(
                f"Exit code         : {returncode}\n"
            )

            fp.write(
                "Expected exit code: "
                f"{', '.join(str(code) for code in expected_returncodes)}\n"
            )

            fp.write(
                f"Result            : {'OK' if result.success else 'FAILED'}\n"
            )

            if expect_stdout_contains:
                fp.write(
                    "Expected in stdout: "
                    f"{'; '.join(expect_stdout_contains)}\n"
                )

            if missing_output:
                fp.write(
                    "MISSING in stdout : "
                    f"{'; '.join(missing_output)}\n"
                )

            if launch_error is not None:
                fp.write(
                    f"Launch error      : {launch_error}\n"
                )

            fp.write("\n")

            self._write_header(
                fp,
                "STDOUT",
            )

            fp.write(stdout)

            if stdout and not stdout.endswith("\n"):
                fp.write("\n")

            fp.write("\n")

            self._write_header(
                fp,
                "STDERR",
            )

            fp.write(stderr)

            if stderr and not stderr.endswith("\n"):
                fp.write("\n")

            fp.write("\n")

        if self.config.verbose:

            prefix = f"[{label}] " if label else ""
            status = "OK" if result.success else "FAILED"

            expected = ", ".join(
                str(code) for code in expected_returncodes
            )

            self.info(
                f"{prefix}  -> {status} ({elapsed:.1f}s, exit={returncode}, "
                f"expected={expected})"
            )

            if missing_output:
                self.info(
                    f"{prefix}  missing from stdout: "
                    f"{'; '.join(missing_output)}"
                )

            if not result.success:

                if stdout.strip():
                    self.info(f"{prefix}  stdout:\n{stdout.strip()}")

                if stderr.strip():
                    self.info(f"{prefix}  stderr:\n{stderr.strip()}")

        return result
