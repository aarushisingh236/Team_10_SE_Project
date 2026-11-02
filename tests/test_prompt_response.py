#!/usr/bin/env python3

import os
import pty
import select
import subprocess
import sys
import termios
import time

COMMAND_COUNT = 20
TIME_LIMIT = 1.0
PROMPT = b"shell> "


def wait_for(master, buffer, token, timeout):
    deadline = time.monotonic() + timeout

    while token not in buffer:
        remaining = deadline - time.monotonic()

        if remaining <= 0:
            raise TimeoutError(f"Timed out waiting for {token!r}")

        ready, _, _ = select.select([master], [], [], remaining)

        if not ready:
            raise TimeoutError(f"Timed out waiting for {token!r}")

        data = os.read(master, 4096)

        if not data:
            raise RuntimeError("Shell closed the terminal unexpectedly")

        buffer += data

    index = buffer.index(token) + len(token)
    return buffer[index:]


def main():
    shell = sys.argv[1] if len(sys.argv) > 1 else "./simple-shell"
    master, slave = pty.openpty()

    settings = termios.tcgetattr(slave)
    settings[3] &= ~termios.ECHO
    termios.tcsetattr(slave, termios.TCSANOW, settings)

    process = subprocess.Popen(
        [shell],
        stdin=slave,
        stdout=slave,
        stderr=slave,
        close_fds=True,
    )
    os.close(slave)

    buffer = b""
    latencies = []

    try:
        buffer = wait_for(master, buffer, PROMPT, TIME_LIMIT)

        for number in range(1, COMMAND_COUNT + 1):
            marker = f"PROMPT_TEST_{number:02}".encode()
            start = time.monotonic()

            os.write(master, b"echo " + marker + b"\n")

            buffer = wait_for(master, buffer, marker, TIME_LIMIT)
            buffer = wait_for(master, buffer, PROMPT, TIME_LIMIT)

            elapsed = time.monotonic() - start
            latencies.append(elapsed)

            print(f"Command {number:02}: {elapsed:.6f} seconds")

        maximum = max(latencies)
        average = sum(latencies) / len(latencies)

        print(f"\nCommands tested: {len(latencies)}")
        print(f"Maximum response time: {maximum:.6f} seconds")
        print(f"Average response time: {average:.6f} seconds")

        if maximum >= TIME_LIMIT:
            print("FAIL: At least one prompt exceeded the 1-second limit.")
            return 1

        print("PASS: All 20 prompts appeared within 1 second.")
        return 0

    except (TimeoutError, RuntimeError, OSError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    finally:
        if process.poll() is None:
            try:
                os.write(master, b"exit\n")
                process.wait(timeout=2)
            except (OSError, subprocess.TimeoutExpired):
                process.kill()
                process.wait()

        os.close(master)


if __name__ == "__main__":
    sys.exit(main())
