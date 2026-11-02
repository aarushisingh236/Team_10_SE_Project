#!/usr/bin/env python3
"""Reliability tests for the simple shell (SH-NF-002)."""

import os
import pty
import re
import select
import subprocess
import sys
import time


def run_case(shell, name, commands, expected, timeout=10):
    try:
        result = subprocess.run(
            [shell],
            input=commands,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=timeout,
            check=False,
        )
    except subprocess.TimeoutExpired:
        print(f"FAIL: {name}: shell exceeded {timeout} seconds")
        return False

    if result.returncode < 0:
        print(f"FAIL: {name}: terminated by signal {-result.returncode}")
        return False

    if result.returncode != 0:
        print(f"FAIL: {name}: exit code {result.returncode}")
        print(result.stdout)
        return False

    missing = [item for item in expected if item not in result.stdout]
    if missing:
        print(f"FAIL: {name}: missing output {missing}")
        print(result.stdout)
        return False

    print(f"PASS: {name}")
    return True


def run_background_reaping_case(shell):
    name = "background child completion and reaping"
    master_fd, slave_fd = pty.openpty()
    process = None

    try:
        process = subprocess.Popen(
            [shell],
            stdin=slave_fd,
            stdout=slave_fd,
            stderr=slave_fd,
            close_fds=True,
        )
        os.close(slave_fd)
        slave_fd = -1

        initial = read_pty_output(master_fd, 2)
        if b"shell>" not in initial:
            print(f"FAIL: {name}: initial prompt not received")
            return False

        os.write(master_fd, b"sleep 0.2 &\n")
        output = read_pty_output(master_fd, 2)

        match = re.search(
            rb"Background process started: PID (\d+)",
            output,
        )
        if not match:
            print(f"FAIL: {name}: background PID was not reported")
            print(output.decode(errors="replace"))
            return False

        pid = int(match.group(1))

        # Keep the shell alive while the background child finishes.
        time.sleep(0.6)

        status = subprocess.run(
            ["ps", "-o", "stat=", "-p", str(pid)],
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=3,
            check=False,
        )

        if status.stderr.strip() or status.returncode not in (0, 1):
            print(
                f"FAIL: {name}: could not inspect PID {pid}: "
                f"{status.stderr.strip()}"
            )
            return False

        process_state = status.stdout.strip()

        if process_state:
            if process_state.startswith("Z"):
                print(f"FAIL: {name}: PID {pid} remains a zombie")
            else:
                print(
                    f"FAIL: {name}: PID {pid} still exists "
                    f"with state {process_state}"
                )
            return False

        if process.poll() is not None:
            print(f"FAIL: {name}: shell exited unexpectedly")
            return False

        os.write(master_fd, b"echo BACKGROUND_REAPING_OK\n")
        recovery = read_pty_output(master_fd, 2)

        if b"BACKGROUND_REAPING_OK" not in recovery:
            print(f"FAIL: {name}: shell did not remain usable")
            print(recovery.decode(errors="replace"))
            return False

        os.write(master_fd, b"exit\n")
        process.wait(timeout=3)

        if process.returncode != 0:
            print(f"FAIL: {name}: shell exited with {process.returncode}")
            return False

        print(f"PASS: {name}")
        return True

    except (OSError, subprocess.TimeoutExpired) as error:
        print(f"FAIL: {name}: {error}")
        return False

    finally:
        if slave_fd >= 0:
            os.close(slave_fd)
        os.close(master_fd)

        if process is not None and process.poll() is None:
            process.kill()
            process.wait()

def read_pty_output(master_fd, timeout):
    output = b""
    deadline = time.monotonic() + timeout

    while time.monotonic() < deadline:
        ready, _, _ = select.select(
            [master_fd], [], [], min(0.1, deadline - time.monotonic())
        )
        if not ready:
            continue

        try:
            chunk = os.read(master_fd, 4096)
        except OSError:
            break

        if not chunk:
            break

        output += chunk

    return output


def run_sigint_case(shell):
    name = "Ctrl+C interrupts foreground command but preserves shell"
    master_fd, slave_fd = pty.openpty()
    process = None

    try:
        process = subprocess.Popen(
            [shell],
            stdin=slave_fd,
            stdout=slave_fd,
            stderr=slave_fd,
            close_fds=True,
        )
        os.close(slave_fd)
        slave_fd = -1

        initial = read_pty_output(master_fd, 2)
        if b"shell>" not in initial:
            print(f"FAIL: {name}: initial prompt not received")
            return False

        os.write(master_fd, b"sleep 5\n")
        time.sleep(0.3)
        os.write(master_fd, b"\x03")

        interrupted_output = read_pty_output(master_fd, 1)

        if process.poll() is not None:
            print(f"FAIL: {name}: shell terminated after Ctrl+C")
            return False

        os.write(master_fd, b"echo SIGINT_RECOVERY_OK\n")
        recovery_output = read_pty_output(master_fd, 2)

        if b"SIGINT_RECOVERY_OK" not in recovery_output:
            print(f"FAIL: {name}: shell did not execute the next command")
            print((interrupted_output + recovery_output).decode(errors="replace"))
            return False

        os.write(master_fd, b"exit\n")
        process.wait(timeout=3)

        if process.returncode != 0:
            print(f"FAIL: {name}: shell exited with {process.returncode}")
            return False

        print(f"PASS: {name}")
        return True

    except (OSError, subprocess.TimeoutExpired) as error:
        print(f"FAIL: {name}: {error}")
        return False

    finally:
        if slave_fd >= 0:
            os.close(slave_fd)
        os.close(master_fd)

        if process is not None and process.poll() is None:
            process.kill()
            process.wait()


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 tests/test_reliability.py ./simple-shell")
        return 2

    shell = sys.argv[1]
    failures = 0
    total = 0

    def check(name, commands, expected, timeout=10):
        nonlocal failures, total
        total += 1
        if not run_case(shell, name, commands, expected, timeout):
            failures += 1

    stress_commands = "".join(
        f"echo RELIABILITY_STRESS_{i:02d}\n"
        for i in range(1, 61)
    ) + "exit\n"

    check(
        "repeated command execution",
        stress_commands,
        ["RELIABILITY_STRESS_01", "RELIABILITY_STRESS_60"],
    )

    check(
        "recovery after negative command inputs",
        'echo "unclosed\n'
        'pes_reliability_missing_command\n'
        'echo RELIABILITY_RECOVERY_OK\n'
        'exit\n',
        ["malformed command input", "RELIABILITY_RECOVERY_OK"],
    )

    check(
        "successful pipeline execution",
        "echo reliability_pipeline_ok | tr a-z A-Z\nexit\n",
        ["RELIABILITY_PIPELINE_OK"],
    )

    check(
        "background command responsiveness",
        "sleep 0.2 &\n"
        "echo RELIABILITY_BACKGROUND_OK\n"
        "exit\n",
        ["RELIABILITY_BACKGROUND_OK"],
    )

    total += 1
    if not run_background_reaping_case(shell):
        failures += 1

    total += 1
    if not run_sigint_case(shell):
        failures += 1

    print(f"\nReliability tests: {total - failures}/{total} passed.")

    if failures:
        print(f"{failures} reliability test(s) failed.")
        return 1

    print("All reliability tests passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
