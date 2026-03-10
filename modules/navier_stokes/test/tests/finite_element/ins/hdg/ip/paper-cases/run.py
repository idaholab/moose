#!/usr/bin/env python3

# Courtesy of interaction with chatgpt-5

import argparse
import os
import subprocess
import sys
from pathlib import Path

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
    if a_solve == "strumpack-lu":
      # Override the A/velocity block preconditioner from STRUMPACK ILU to STRUMPACK LU.
      cmd.append("Preconditioning/FSP/u/petsc_options='-ksp_converged_reason'")
      cmd.append(
          "Preconditioning/FSP/u/petsc_options_iname="
          "'-pc_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side "
          "-pc_factor_mat_solver_type -ksp_max_it -ksp_atol -ksp_norm_type'"
      )
      cmd.append(
          "Preconditioning/FSP/u/petsc_options_value="
          "'lu gmres 1e-2 300 right strumpack 300 1e-8 unpreconditioned'"
      )

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
        default="strumpack-ilu",
        help=(
            "A/velocity block solve strategy: 'strumpack-ilu' (default) "
            "or 'strumpack-lu'."
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
