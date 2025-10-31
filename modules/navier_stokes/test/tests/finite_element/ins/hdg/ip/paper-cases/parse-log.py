#!/usr/bin/env python3

# Courtesy chatgpt-5

import re
import sys
import argparse
import pandas as pd
from typing import Optional

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

        def _find_max_iterations(block: str, solver_name: str) -> int:
            """
            Return the maximum 'iterations N' seen for lines like:
              Linear <solver_name> solve converged due to ... iterations N
              Linear <solver_name> solve did not converge due to ... iterations N
            """
            pat = re.compile(
                rf"^\s*Linear\s+{re.escape(solver_name)}\s+solve\s+(?:converged|did\s+not\s+converge)\s+due\s+to\s+\S+\s+iterations\s+(\d+)\s*$",
                re.IGNORECASE | re.MULTILINE
            )
            vals = [int(x) for x in pat.findall(block)]
            return max(vals) if vals else 0

        max_outer = _find_max_iterations(block, "nl0_condensed_")
        max_p     = _find_max_iterations(block, "nl0_condensed_fieldsplit_p_")
        max_u     = _find_max_iterations(block, "nl0_condensed_fieldsplit_u_")

        rows.append({
            "Re": tval,
            "nl": nonlinear_its,
            "o": max_outer,
            "p": max_p,
            "u": max_u,
        })

    return pd.DataFrame(rows).sort_values("Re").reset_index(drop=True)

def parse_header_for_key(key: str, text: str) -> Optional[int]:
    """
    Extract selected MOOSE header information
    """
    pattern = rf"^\s*{re.escape(key)}:\s*([0-9]+)\s*$"
    m = re.search(pattern, text, re.MULTILINE)
    return int(m.group(1)) if m else None

def parse_num_dofs(text: str) -> Optional[int]:
    return parse_header_for_key("Num DOFs", text)

def parse_num_elems(text: str) -> Optional[int]:
    # Match either "Elems: <num>" or "Elems:" followed by "Total: <num>"
    pattern = (
        r"^\s*Elems:\s*(?:(?P<single>\d+)\s*$"              # single-process
        r"|(?:\n\s*Total:\s*(?P<multi>\d+)\s*$))"           # multi-process
    )
    m = re.search(pattern, text, re.MULTILINE)
    if not m:
        return None
    return int(m.group("single") or m.group("multi"))

def parse_perf_solve_avg(text: str) -> Optional[float]:
    """
    From the Performance Graph, find the line that starts with
    '|   NavierStokesProblem::solve' and return the SECOND 'Avg(s)'
    (the one in the Total/Avg/%/Mem group).
    """
    # Locate the performance graph section (optional, robust if the whole log is passed)
    # Then capture the specific row; split by '|' and take the second Avg(s) = column index 8.
    pat = re.compile(r"^\|\s*NavierStokesProblem::solve.*$", re.MULTILINE)
    m = pat.search(text)
    if not m:
        return None
    row = m.group(0)
    parts = [p.strip() for p in row.split("|")]
    # Expected layout:
    # 0:'' 1:Section 2:Calls 3:Self(s) 4:Avg(s) 5:% 6:Mem(MB) 7:Total(s) 8:Avg(s) 9:% 10:Mem(MB) 11:''
    if len(parts) >= 9 and parts[8]:
        try:
            return float(parts[8])
        except ValueError:
            return None
    return None

def parse_mat_aij_rows(mat_name: str, text: str) -> Optional[int]:
    # Regex requirements:
    # 1) Exact line:  Mat Object: (<name>) ...
    # 2) Immediately following line:  type: .*aij
    # 3) Later line (same block): rows=<N>
    pattern = re.compile(
        rf"^\s*Mat Object:\s*\(\s*{re.escape(mat_name)}\s*\).*\n"  # Mat Object line
        rf"^\s*type:\s*\S*?aij\s*$"                             # next line: type ... aij
        rf"(?P<body>[\s\S]*?)"                                  # capture rest of block segment
        rf"^\s*rows\s*=\s*(?P<rows>[0-9]+)\b",                  # rows=NNN
        re.IGNORECASE | re.MULTILINE
    )
    m = pattern.search(text)
    if not m:
        return None
    try:
        return int(m.group("rows"))
    except ValueError:
        return None

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

    if args.times:
        times = [float(s) for s in args.times.split(",")]
    else:
        times = [1, 10, 100, 1000, 5000, 10000]

    df = parse_moose_petsc_log(text, times=times)

    num_elems = parse_num_elems(text)
    num_dofs = parse_num_dofs(text)
    num_condensed_dofs = parse_mat_aij_rows("nl0_condensed_fieldsplit_u_", text)
    solve_avg = parse_perf_solve_avg(text)

    if args.csv:
        # Keep CSV clean; write summary to stderr so CSV parsers aren’t confused
        if num_elems is not None:
            sys.stderr.write(f"Num Elems: {num_elems}\n")
        if num_dofs is not None:
            sys.stderr.write(f"Num DOFs: {num_dofs}\n")
        if num_condensed_dofs is not None:
            sys.stderr.write(f"Num Condensed DOFs: {num_condensed_dofs}\n")
        if solve_avg is not None:
            sys.stderr.write(f"NavierStokesProblem::solve Avg(s): {solve_avg}\n")
        print(df.to_csv(index=False))
    else:
        if num_elems is not None:
            print(f"Num Elems: {num_elems}")
        if num_dofs is not None:
            print(f"Num DOFs: {num_dofs}")
        if num_condensed_dofs is not None:
            print(f"Num Condensed DOFs: {num_condensed_dofs}")
        if solve_avg is not None:
            print(f"NavierStokesProblem::solve Avg(s): {solve_avg}")
        # Pretty print
        try:
            from tabulate import tabulate
            print(tabulate(df, headers="keys", tablefmt="github", showindex=False))
        except Exception:
            # Fallback if tabulate not available
            print(df.to_string(index=False))

if __name__ == "__main__":
    main()
