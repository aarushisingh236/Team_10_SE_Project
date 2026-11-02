#!/usr/bin/env python3
"""Integration test for SH-F-017: background process PID reporting."""

import os
import pty
import re
import select
import signal
import subprocess
import sys
import time


def read_until(master_fd, pattern, timeout=3):
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

        if re.search(pattern, output):
            return output

    return output


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 tests/test_background_pid.py ./simple-shell")
        return 2

    shell = sys.argv[1]
    master_fd, slave_fd = pty.openpty()
    process = None
    child_pid = None

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

        initial = read_until(master_fd, rb"shell>")
        if b"shell>" not in initial:
            print("FAIL: shell prompt was not received")
            return 1

        os.write(master_fd, b"sleep 10 &\n")

        output = read_until(
            master_fd,
            rb"Background process started: PID \d+",
        )

        match = re.search(
            rb"Background process started: PID (\d+)",
            output,
        )

        if not match:
            print("FAIL: shell did not display the background PID")
            print(output.decode(errors="replace"))
            return 1

        child_pid = int(match.group(1))
        print(f"Reported background PID: {child_pid}")

        if child_pid == process.pid:
            print("FAIL: displayed PID is the shell's PID, not its child")
            return 1

        if process.poll() is not None:
            print("FAIL: shell exited unexpectedly")
            return 1

        parent_result = subprocess.run(
            ["ps", "-o", "ppid=", "-p", str(child_pid)],
            capture_output=True,
            text=True,
            timeout=3,
            check=False,
        )

        if parent_result.returncode != 0 or not parent_result.stdout.strip():
            print("FAIL: reported PID does not identify a running process")
            return 1

        child_parent_pid = int(parent_result.stdout.strip())

        if child_parent_pid != process.pid:
            print(
                f"FAIL: PID {child_pid} has parent {child_parent_pid}, "
                f"expected shell PID {process.pid}"
            )
            return 1

        command_result = subprocess.run(
            ["ps", "-o", "comm=", "-p", str(child_pid)],
            capture_output=True,
            text=True,
            timeout=3,
            check=False,
        )

        command_name = os.path.basename(command_result.stdout.strip())

        if command_result.returncode != 0 or command_name != "sleep":
            print(
                f"FAIL: PID {child_pid} is not running the expected "
                f"sleep command (found: {command_name or 'unknown'})"
            )
            return 1

        print(f"PASS: PID {child_pid} identifies the sleep child process")
        print(f"PASS: Child PPID {child_parent_pid} matches shell PID {process.pid}")
        print("All SH-F-017 checks passed.")
        return 0

    except (OSError, ValueError, subprocess.TimeoutExpired) as error:
        print(f"FAIL: {error}")
        return 1

    finally:
        if child_pid is not None:
            try:
                os.kill(child_pid, signal.SIGTERM)
            except ProcessLookupError:
                pass

        if process is not None and process.poll() is None:
            try:
                os.write(master_fd, b"exit\n")
                process.wait(timeout=3)
            except (OSError, subprocess.TimeoutExpired):
                process.kill()
                process.wait()

        if slave_fd >= 0:
            os.close(slave_fd)

        os.close(master_fd)


if __name__ == "__main__":
    sys.exit(main())
