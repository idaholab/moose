import argparse
import re
from pathlib import Path

import matplotlib.pyplot as plt
# RUN SIMULATION WITH : ../otter-opt -i yourfile.i 2>&1 | tee yourfile.log
# RUN WITH : python plot_residuals.py yourfile.log

ANSI_RE = re.compile(r'\x1b\[[0-9;]*m')
FLOAT_RE = r'[-+]?(?:\d*\.\d+|\d+\.?)(?:[eE][-+]?\d+)?'

PATTERNS = {
    "u momentum": re.compile(rf'^\s*Momentum equation:\s*Component\s*1\s+({FLOAT_RE})\b'),
    "v momentum": re.compile(rf'^\s*Momentum equation:\s*Component\s*2\s+({FLOAT_RE})\b'),
    "pressure": re.compile(rf'^\s*Pressure equation:\s+({FLOAT_RE})\b'),
    "energy": re.compile(rf'^\s*Advected system:\s*\S+\s+({FLOAT_RE})\b'),
    "solid": re.compile(rf'^\s*(?:Currently Executing\s+)?Solid energy equation:\s+({FLOAT_RE})\b'),
}

ITER_RE = re.compile(r'^\s*Iteration\s+(\d+)\b')


def strip_ansi(text):
    return ANSI_RE.sub('', text)


def parse_log(log_path):
    series = {name: {"iters": [], "res": []} for name in PATTERNS}
    current_it = None
    unparsed = []

    with open(log_path, "r") as f:
        for lineno, raw_line in enumerate(f, start=1):
            line = strip_ansi(raw_line).strip()
            if not line:
                continue

            m_it = ITER_RE.match(line)
            if m_it:
                current_it = int(m_it.group(1))
                continue

            matched = False
            for name, pattern in PATTERNS.items():
                m = pattern.match(line)
                if m:
                    if current_it is None:
                        unparsed.append((lineno, "Residual found before first iteration", line))
                    else:
                        series[name]["iters"].append(current_it)
                        series[name]["res"].append(float(m.group(1)))
                    matched = True
                    break

            if (not matched and
                any(key in line for key in [
                    "Momentum equation:",
                    "Pressure equation:",
                    "Advected system:",
                    "Solid energy equation:"
                ])):
                unparsed.append((lineno, "Unmatched residual-like line", line))

    return series, unparsed


def plot_series(series, output_file="all_residuals.png", show=True):
    plt.figure()

    plotted_any = False
    for name, data in series.items():
        if data["iters"] and data["res"]:
            plt.semilogy(data["iters"], data["res"], label=name)
            plotted_any = True

    if not plotted_any:
        raise RuntimeError("No residual data found to plot.")

    plt.xlabel("Iteration")
    plt.ylabel("Residual (log scale)")
    plt.title("Residuals vs Iteration")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(output_file, dpi=150)

    if show:
        plt.show()


def main():
    parser = argparse.ArgumentParser(description="Plot residuals from a MOOSE/Otter log file.")
    parser.add_argument("logfile", nargs="?", default="step4s.log", help="Path to log file")
    parser.add_argument("-o", "--output", default="all_residuals.png", help="Output image filename")
    parser.add_argument("--no-show", action="store_true", help="Do not display the plot window")
    parser.add_argument("--report-unparsed", action="store_true", help="Print lines that look like residuals but were not parsed")
    args = parser.parse_args()

    series, unparsed = parse_log(args.logfile)

    print("Parsed residual counts:")
    for name, data in series.items():
        print(f"  {name:>10}: {len(data['res'])}")

    if args.report_unparsed and unparsed:
        print("\nUnparsed residual-like lines:")
        for lineno, reason, line in unparsed:
            print(f"  line {lineno}: {reason}: {line}")

    plot_series(series, args.output, show=not args.no_show)
    print(f"\nSaved plot to {args.output}")


if __name__ == "__main__":
    main()