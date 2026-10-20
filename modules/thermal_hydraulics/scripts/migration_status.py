#!/usr/bin/env python3
"""Report which classic Component tests have an ActionComponent counterpart.

Compares test/tests/components/*/tests against test/tests/actioncomponents/*/tests. A classic test
is matched to an ActionComponent test in the directory of the same name when both run the same
input file with the same 'cli_args' (after renaming 'Components/' to 'ActionComponents/' in the
classic arguments). This is a static check of the test specifications; it does not run anything,
so a match means a counterpart exists, not that it passes (run the tests for that).

Status of each classic test:
  identical   an ActionComponent test runs the same input with the same arguments, and the two
              input files are identical apart from the block name (a pure block switch)
  modified    an ActionComponent test runs the same input, but the test differs from the classic
              one: either the input files differ in more than the block name, or the tests use
              different 'cli_args'
  missing     no ActionComponent test runs this input (in test/tests/actioncomponents/<directory>,
              which may not exist)
"""

import argparse
import sys
from collections import Counter, OrderedDict
from pathlib import Path

THM_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(THM_DIR.parents[1] / "python"))
from pyhit import pyhit  # noqa: E402

TESTS = THM_DIR / "test" / "tests"
# ANSI colors; 256-color orange since the basic "yellow" renders as brown in many terminals
GREEN, ORANGE, RED, RESET = "\033[32m", "\033[38;5;208m", "\033[31m", "\033[0m"
# Color of the nonzero counts in each summary column
COLUMN_COLORS = {"identical": GREEN, "modified": ORANGE, "missing": RED}
STATUSES = ("identical", "modified", "missing")


def normalize_args(args):
    """Order-independent form of cli_args, with the classic block name mapped to the AC one."""
    args = (args or "").replace('"', " ").replace("'", " ")
    return tuple(
        sorted(
            a.replace("Components/", "ActionComponents/").replace(
                "ActionActionComponents/", "ActionComponents/"
            )
            for a in args.split()
        )
    )


def leaf_tests(tests_file):
    """(path, type, input, normalized cli_args) of each test; group parameters are inherited."""
    root = pyhit.load(str(tests_file))
    found = []

    def walk(node, inherited, path):
        params = dict(inherited)
        params.update({k: v for k, v in node.params()})
        if "input" in params and "type" in params and not node.children:
            found.append(
                (
                    "/".join(path + [node.name]),
                    params["type"],
                    params["input"].strip(),
                    normalize_args(params.get("cli_args")),
                )
            )
        for child in node.children:
            walk(
                child,
                params,
                path if node is root or node.name == "Tests" else path + [node.name],
            )

    walk(root, {}, [])
    return found


def collect(kind):
    out = OrderedDict()
    for tests_file in sorted((TESTS / kind).glob("*/tests")):
        out[tests_file.parent.name] = leaf_tests(tests_file)
    return out


def body(path, ac):
    """Input file lines without comments or blanks, with the block name unified, in sorted order.

    '!include' lines are replaced by the (recursively expanded) contents of the file they name,
    so tests that share their input between the two modes through wrapper files compare equal.
    """
    lines = []
    for line in path.read_text().splitlines():
        line = line.split("#")[0].strip()
        if line.startswith("!include"):
            included = (path.parent / line.split(None, 1)[1].strip()).resolve()
            lines.extend(body(included, ac))
        elif line:
            lines.append(
                line.replace("[ActionComponents]", "[Components]") if ac else line
            )
    return sorted(lines)


def same_input(name, inp):
    classic, ac = (
        TESTS / "components" / name / inp,
        TESTS / "actioncomponents" / name / inp,
    )
    return classic.exists() and ac.exists() and body(classic, False) == body(ac, True)


def classify(name, classic, ac):
    ac_keys = {(inp, args) for _, _, inp, args in ac}
    ac_inputs = {inp for _, _, inp, _ in ac}
    rows = []
    for path, ttype, inp, args in classic:
        if (inp, args) in ac_keys:
            status = "identical" if same_input(name, inp) else "modified"
        elif inp in ac_inputs:
            status = "modified"
        else:
            status = "missing"
        rows.append((path, ttype, inp, status))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument(
        "-v", "--verbose", action="store_true", help="list every classic test"
    )
    parser.add_argument(
        "--markdown", action="store_true", help="markdown tables for a PR description"
    )
    parser.add_argument(
        "--only", choices=STATUSES, help="with -v, list only tests with this status"
    )
    parser.add_argument(
        "--color",
        choices=("auto", "always", "never"),
        default="auto",
        help="color the summary table (default: only when writing to a terminal)",
    )
    args = parser.parse_args()
    use_color = args.color == "always" or (args.color == "auto" and sys.stdout.isatty())

    classic, ac = collect("components"), collect("actioncomponents")
    summary = OrderedDict()
    details = OrderedDict()
    for name, tests in classic.items():
        rows = classify(name, tests, ac.get(name, []))
        details[name] = rows
        summary[name] = Counter(r[3] for r in rows)
    total = Counter()
    for c in summary.values():
        total.update(c)

    cols = ["identical", "modified", "missing"]
    n_all = sum(total.values())
    if args.markdown:
        print("| directory | tests | " + " | ".join(cols) + " |")
        print("|---|---|" + "---|" * len(cols))
        for name, c in summary.items():
            if name in ac or args.verbose:
                print(
                    f"| {name} | {sum(c.values())} | "
                    + " | ".join(str(c[k]) for k in cols)
                    + " |"
                )
        print(
            f"| **total** | **{n_all}** | "
            + " | ".join(f"**{total[k]}**" for k in cols)
            + " |"
        )
    else:
        width = max(len(n) for n in summary)

        def paint(text, color):
            return f"{color}{text}{RESET}" if use_color and color else text

        def row(label, count, c):
            # Pad before coloring so the escape codes do not disturb the column alignment
            if c["missing"]:
                label_color = RED
            elif c["modified"]:
                label_color = ORANGE
            else:
                label_color = GREEN
            cells = [
                paint(f"{c[k]:>11}", COLUMN_COLORS.get(k) if c[k] else None)
                for k in cols
            ]
            return (
                f'{paint(f"{label:<{width}}", label_color)}  {count:>5}  '
                + "  ".join(cells)
            )

        print(f'{"directory":<{width}}  tests  ' + "  ".join(f"{k:>11}" for k in cols))
        for name, c in summary.items():
            print(row(name, sum(c.values()), c))
        print(row("TOTAL", n_all, total))
        done = total["identical"] + total["modified"]
        print(
            f"\n{done}/{n_all} classic tests have an ActionComponent counterpart "
            f'({total["identical"]} identical inputs, {total["modified"]} modified).'
        )

    if args.verbose:
        print()
        for name, rows in details.items():
            shown = [r for r in rows if not args.only or r[3] == args.only]
            if shown:
                print(f"{name}/")
                for path, ttype, inp, status in shown:
                    print(f"  {status:<11} {path}  [{ttype}: {inp}]")

    # AC tests with no classic counterpart (new tests, or ones whose inputs were rewritten)
    orphans = []
    for name, tests in ac.items():
        classic_keys = {(inp, args) for _, _, inp, args in classic.get(name, [])}
        for path, ttype, inp, a in tests:
            if (inp, a) not in classic_keys:
                orphans.append(f"{name}/{path}  [{ttype}: {inp}]")
    if orphans:
        print(f"\nActionComponent tests with no classic counterpart ({len(orphans)}):")
        for o in orphans:
            print("  " + o)


if __name__ == "__main__":
    main()
