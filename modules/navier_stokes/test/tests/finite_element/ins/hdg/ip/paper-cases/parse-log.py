#!/usr/bin/env python3

# Courtesy chatgpt-5

import re
import sys
import argparse
import pandas as pd

def parse_moose_petsc_log(text: str, times=None) -> pd.DataFrame:
    """
    Parse a MOOSE/PETSc log and extract, per requested time value:
      - nonlinear_its
      - max_ksp_outer_its (nl0_condensed_)
      - max_p_its (nl0_condensed_fieldsplit_p_)
      - max_u_its (nl0_condensed_fieldsplit_u_)
    """
    text = text.replace('\r\n', '\n').replace('\r', '\n')

    block_pattern = re.compile(
        r"Time Step\s+\d+,\s*time\s*=\s*([0-9.eE+\-]+).*?(?=Time Step\s+\d+,\s*time\s*=|\Z)",
        re.DOTALL,
    )
    blocks = [(float(m.group(1)), m.group(0)) for m in block_pattern.finditer(text)]

    wanted = None
    if times is not None:
        def to_float(x):
            return float(str(x).strip())
        wanted = set(to_float(t) for t in times)

    rows = []
    for tval, block in blocks:
        if wanted is not None and tval not in wanted:
            continue

        # Nonlinear iterations: take the *last* "X Nonlinear ..." index
        # that appears *before* 'Solve Converged!' (not a count of lines).
        solve_idx = block.find("Solve Converged!")
        scan_text = block[:solve_idx] if solve_idx != -1 else block
        nl_matches = list(re.finditer(r"^\s*(\d+)\s+Nonlinear\b", scan_text, re.MULTILINE))
        nonlinear_its = int(nl_matches[-1].group(1)) if nl_matches else 0

        outer_matches = re.findall(
            r"Linear\s+nl0_condensed_\s+solve\s+converged.*?iterations\s+(\d+)",
            block, re.IGNORECASE
        )
        max_outer = max((int(x) for x in outer_matches), default=0)

        p_matches = re.findall(
            r"Linear\s+nl0_condensed_fieldsplit_p_\s+solve\s+converged.*?iterations\s+(\d+)",
            block, re.IGNORECASE
        )
        u_matches = re.findall(
            r"Linear\s+nl0_condensed_fieldsplit_u_\s+solve\s+converged.*?iterations\s+(\d+)",
            block, re.IGNORECASE
        )
        max_p = max((int(x) for x in p_matches), default=0)
        max_u = max((int(x) for x in u_matches), default=0)

        rows.append({
            "Re": tval,
            "nl": nonlinear_its,
            "o": max_outer,
            "p": max_p,
            "u": max_u,
        })

    return pd.DataFrame(rows).sort_values("Re").reset_index(drop=True)

def main():
    ap = argparse.ArgumentParser(description="Parse PETSc/MOOSE log for iteration stats by time.")
    ap.add_argument("-i", "--input", type=str, help="Path to log file (default: stdin).")
    ap.add_argument("-t", "--times", type=str,
                    help="Comma-separated time values to include (e.g. '1,10'). "
                         "If omitted, all times found are reported.")
    ap.add_argument("--csv", action="store_true", help="Output CSV instead of a table.")
    args = ap.parse_args()

    if args.input:
        with open(args.input, "r", encoding="utf-8") as f:
            text = f.read()
    else:
        text = sys.stdin.read()

    times = None
    if args.times:
        times = [float(s) for s in args.times.split(",")]

    df = parse_moose_petsc_log(text, times=times)

    if args.csv:
        print(df.to_csv(index=False))
    else:
        # Pretty print
        try:
            from tabulate import tabulate
            print(tabulate(df, headers="keys", tablefmt="github", showindex=False))
        except Exception:
            # Fallback if tabulate not available
            print(df.to_string(index=False))

if __name__ == "__main__":
    main()
