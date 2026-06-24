#!/usr/bin/env python3

# Courtesy of interaction with chatgpt-5

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

# Resolve through the per-case symlink so this points at the real paper-cases
# directory regardless of which case directory run.py was launched from.
PRECON_FILE = Path(__file__).resolve().parent / "fs-plus-strumpack-preconditioner.i"

def read_u_petsc_options(precon_file):
    """
    Return (iname, value) lists for the [u] block's petsc_options_iname and
    petsc_options_value in the given preconditioner input file. Reading them
    here avoids duplicating the full option strings in this script.
    """
    text = precon_file.read_text(encoding="utf-8")
    block = re.search(r"^\s*\[u\]\s*$(.*?)^\s*\[\]\s*$", text, re.DOTALL | re.MULTILINE)
    if not block:
        raise RuntimeError(f"Could not find [u] block in {precon_file}")
    body = block.group(1)

    def _vec(name):
        m = re.search(rf"{name}\s*=\s*'([^']*)'", body)
        if not m:
            raise RuntimeError(f"Could not find {name} in [u] block of {precon_file}")
        return m.group(1).split()

    return _vec("petsc_options_iname"), _vec("petsc_options_value")

def run_case(exec_path,
             input_file,
             procs,
             refine,
             set_schur_pre,
             num_steps,
             disc,
             profile,
             base_n,
             a_solve):
    input_base = input_file[:-2]
    tag = f"{input_base}-{procs}proc-{refine}refine-{set_schur_pre}-pre-{disc}-{a_solve}"
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
    if base_n is not None:
      cmd.append(f"n={base_n}")
    # Set the A/velocity block pc_type explicitly so the strategy does not depend
    # on the input file. The only difference between the STRUMPACK ILU and LU
    # strategies is pc_type; the remaining options are read from the [u] block.
    iname, value = read_u_petsc_options(PRECON_FILE)
    value[iname.index("-pc_type")] = "lu" if a_solve == "strumpack-lu" else "ilu"
    cmd.append("Preconditioning/FSP/u/petsc_options_value='%s'" % " ".join(value))

    # Prepare environment (set MOOSE_PROFILE_BASE per run)
    env = os.environ.copy()
    if profile:
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
        description="Run Navier-Stokes cases with scaling refine/procs and logging."
    )
    parser.add_argument(
        "--exec", required=True,
        help="Path to the MOOSE executable (required)."
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
        "--num-steps", type=int, nargs="+",
        help=(
            "Executioner steps per case. Provide one value to use for all cases, "
            "or one value per case."
        )
    )
    parser.add_argument(
        "--disc",
        choices=["edghdg", "hdg"],
        required=True,
        help="Discretization type (choose 'edghdg' or 'hdg')"
    )
    parser.add_argument(
        "--profile", type=bool, default=False,
        help="Whether to profile (default: %(default)s)"
    )
    parser.add_argument(
        "--base-n", type=int,
        help="Base mesh n value; passed as Mesh/n when provided."
    )
    parser.add_argument(
        "--A-solve",
        choices=["strumpack-ilu", "strumpack-lu"],
        default="strumpack-lu",
        help=(
            "A/velocity block solve strategy: 'strumpack-lu' (default) "
            "or 'strumpack-ilu'."
        ),
    )

    args = parser.parse_args()

    if args.num_cases > 1 and args.proc_multiplier is None:
        parser.error("--proc-multiplier is required when --num-cases > 1")
    if args.num_steps is not None and len(args.num_steps) not in (1, args.num_cases):
        parser.error(
            "--num-steps must contain either one value or exactly --num-cases values"
        )

    exec_path = args.exec
    input_file = args.input
    procs = args.start_procs
    refine = args.start_refine

    for _case in range(1, args.num_cases + 1):
        if args.num_steps is None:
            case_num_steps = None
        elif len(args.num_steps) == 1:
            case_num_steps = args.num_steps[0]
        else:
            case_num_steps = args.num_steps[_case - 1]

        run_case(exec_path,
                 input_file,
                 procs,
                 refine,
                 args.set_schur_pre,
                 case_num_steps,
                 args.disc,
                 args.profile,
                 args.base_n,
                 args.A_solve)

        if _case < args.num_cases:
            procs *= args.proc_multiplier
            refine += 1


if __name__ == "__main__":
    main()
