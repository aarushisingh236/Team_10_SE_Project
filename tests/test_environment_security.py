import os
import subprocess
import sys

shell = sys.argv[1] if len(sys.argv) > 1 else "./simple-shell"

secret = "PRIVATE_SECRET_MARKER_" + ("S" * 5000)
sentinel = "UNRELATED_ENVIRONMENT_SENTINEL"

env = os.environ.copy()
env["SH_SR_004_SECRET"] = secret
env["SH_SR_004_SENTINEL"] = sentinel
env["SH_SR_004_PUBLIC"] = "individual-value-allowed"

commands = (
    "echo $SH_SR_004_PUBLIC\n"
    "echo $SH_SR_004_SECRET\n"
    "exit\n"
)

result = subprocess.run(
    [shell],
    input=commands,
    text=True,
    capture_output=True,
    env=env,
    timeout=5,
)

output = result.stdout + result.stderr

if "individual-value-allowed" not in output:
    raise SystemExit("FAIL: individual variable expansion stopped working")

if "SH_SR_004_SECRET" not in output:
    raise SystemExit("FAIL: error did not identify the relevant variable")

if secret in output or "PRIVATE_SECRET_MARKER_" in output:
    raise SystemExit("FAIL: secret environment value appeared in shell output")

if sentinel in output:
    raise SystemExit("FAIL: unrelated environment variable appeared in output")

print("Environment disclosure integration test passed.")
