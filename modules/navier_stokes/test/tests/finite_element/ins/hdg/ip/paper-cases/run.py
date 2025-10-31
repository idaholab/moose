#!/usr/bin/env python3

# Courtesy of interaction with chatgpt-5

import argparse
import os
import subprocess
import sys
from pathlib import Path
import signal

def _preexec_detach():
    # Start a new session (like setsid) and ignore SIGHUP so the process
    # survives terminal logout/disconnects.
    os.setsid()
    signal.signal(signal.SIGHUP, signal.SIG_IGN)

def run_case(exec_path, input_file, procs, refine, set_schur_pre, num_steps, detach):
    input_base = input_file[:-2]
    tag = f"{input_base}-{procs}proc-{refine}refine"
    if not detach:
      print(f"=== Running case: {procs} ranks, refine={refine} ===\n", flush=True)

    # Build command (mirrors the bash command)
    cmd = [
        "mpiexec", "-np", str(procs),
        exec_path,
        "-i", input_file,
        f"Mesh/uniform_refine={refine}",
        f"Outputs/file_base={tag}",
        f"Problem/set_schur_pre={set_schur_pre}",
        "--color", "off",
    ]
    if num_steps is not None:
      cmd.append(f"Executioner/num_steps={num_steps}")

    # Prepare environment (set MOOSE_PROFILE_BASE per run)
    env = os.environ.copy()
    env["MOOSE_PROFILE_BASE"] = tag

    log_path = Path(f"{tag}.log")
    if detach:
        with log_path.open("w", encoding="utf-8") as logfile:
            logfile.write(f"## COMMAND: {' '.join(map(str, cmd))}\n\n")
            logfile.flush()
            proc = subprocess.Popen(
                cmd,
                stdout=logfile,
                stderr=subprocess.STDOUT,
                stdin=subprocess.DEVNULL,
                env=env,
                text=True,
                close_fds=True,
                preexec_fn=_preexec_detach,
            )
        print(f"=== Launched detached case with PID={proc.pid}: {tag} ===\n", flush=True)
        return
    else:
        with log_path.open("w", encoding="utf-8") as logfile:
            logfile.write(f"## COMMAND: {' '.join(map(str, cmd))}\n\n")
            logfile.flush()
            proc = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                env=env,
                text=True,
                bufsize=1,
                universal_newlines=True,
            )
            assert proc.stdout is not None
            for line in proc.stdout:
                sys.stdout.write(line)
                logfile.write(line)
            ret = proc.wait()

        if ret != 0:
            print(f"\nCommand failed for tag '{tag}' with exit code {ret}", file=sys.stderr)
            sys.exit(ret)

        print(f"\n=== Finished case: {tag} ===\n", flush=True)

def require_i_suffix(value: str) -> str:
    if not value.endswith(".i"):
        raise argparse.ArgumentTypeError("Input file must end with '.i'")
    return value

def main():
    parser = argparse.ArgumentParser(
        description="Run Navier–Stokes cases with scaling refine/procs and logging."
    )
    parser.add_argument(
        "--exec", default="/data/lindad/projects/moose4/modules/navier_stokes/navier_stokes-oprof",
        help="Path to the MOOSE executable (default: %(default)s)"
    )
    parser.add_argument(
        "--input", required=True, type=require_i_suffix,
        help="Input file (required)."
    )
    parser.add_argument(
        "--num-cases", type=int, default=3,
        help="Number of cases to run (default: %(default)s)"
    )
    parser.add_argument(
        "--start-procs", type=int, default=1,
        help="Starting MPI ranks (default: %(default)s)"
    )
    parser.add_argument(
        "--start-refine", type=int, default=0,
        help="Starting uniform refine level (default: %(default)s)"
    )
    parser.add_argument(
        "--proc-multiplier", type=int, required=True,
        help="Multiplicative increase in ranks per case (required)."
    )
    parser.add_argument(
        "--set-schur-pre", required=True,
        help="Value for Problem/set_schur_pre (required)."
    )
    parser.add_argument(
        "--num-steps", type=int,
        help="Number of executioner steps to run (default: %(default)s)"
    )
    parser.add_argument(
        "--detach", action="store_true",
        help="Run each case detached (new session, ignores SIGHUP; logs only to <tag>.log)."
    )

    args = parser.parse_args()

    exec_path = args.exec
    input_file = args.input
    procs = args.start_procs
    refine = args.start_refine

    for _case in range(1, args.num_cases + 1):
        run_case(exec_path,
                 input_file,
                 procs,
                 refine,
                 args.set_schur_pre,
                 args.num_steps,
                 args.detach)

        procs *= args.proc_multiplier
        refine += 1


if __name__ == "__main__":
    main()
