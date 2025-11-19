#!/usr/bin/env python3

# Courtesy of interaction with chatgpt-5

import argparse
import os
import subprocess
import sys
from pathlib import Path

def run_case(exec_path, input_file, procs, refine, set_schur_pre, num_steps, disc):
    input_base = input_file[:-2]
    tag = f"{input_base}-{procs}proc-{refine}refine-{set_schur_pre}-pre"
    print(f"=== Running case: {procs} ranks, refine={refine} ===\n", flush=True)
    if disc == "edghdg":
        vel_face_fe_type = "LAGRANGE"
    else:
        vel_face_fe_type = "SIDE_HIERARCHIC"

    # Build command (mirrors the bash command)
    cmd = [
        "mpiexec", "-np", str(procs),
        exec_path,
        "-i", input_file,
        f"Mesh/uniform_refine={refine}",
        f"Outputs/file_base={tag}",
        f"Problem/set_schur_pre={set_schur_pre}",
        f"Variables/vel_bar_x/family={vel_face_fe_type}",
        f"Variables/vel_bar_y/family={vel_face_fe_type}",
        "--color", "off",
    ]
    if num_steps is not None:
      cmd.append(f"Executioner/num_steps={num_steps}")

    # Prepare environment (set MOOSE_PROFILE_BASE per run)
    env = os.environ.copy()
    env["MOOSE_PROFILE_BASE"] = tag

    log_path = Path(f"{tag}.log")

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
        "--proc-multiplier", type=int,
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
        "--disc",
        choices=["edghdg", "hdg"],
        required=True,
        help="Discretization type (choose 'edghdg' or 'hdg')"
    )

    args = parser.parse_args()

    if args.num_cases > 1 and args.proc_multiplier is None:
        parser.error("--proc-multiplier is required when --num-cases > 1")

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
                 args.disc)

        if _case < args.num_cases:
            procs *= args.proc_multiplier
            refine += 1


if __name__ == "__main__":
    main()
