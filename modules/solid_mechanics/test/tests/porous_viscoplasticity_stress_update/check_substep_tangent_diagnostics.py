#!/usr/bin/env python3

import argparse
import csv
import glob
import os
import sys


COUNTERS = (
    "substep_tangent_evaluations",
    "substep_tangent_perturbed_replays",
    "substep_tangent_restoration_replays",
    "substep_tangent_centered_directions",
    "substep_tangent_one_sided_directions",
    "substep_tangent_failed_replays",
    "substep_tangent_branch_mismatches",
    "substep_tangent_cross_branch_directions",
    "substep_tangent_restoration_branch_mismatches",
)


def matching_files(prefix):
    return sorted(glob.glob(f"{prefix}_*_rank*_thread*_instance*.csv"))


def clean(prefix):
    for path in matching_files(prefix):
        os.remove(path)


def read_counters(prefix):
    totals = {name: 0 for name in COUNTERS}
    files = matching_files(prefix)
    if not files:
        raise RuntimeError(f"no performance diagnostic files found for prefix {prefix!r}")

    for path in files:
        values = {}
        with open(path, newline="") as stream:
            for row in csv.reader(stream):
                if len(row) == 2:
                    values[row[0]] = row[1]
        missing = [name for name in COUNTERS if name not in values]
        if missing:
            raise RuntimeError(f"{path} is missing counters: {', '.join(missing)}")
        for name in COUNTERS:
            totals[name] += int(values[name])

    return files, totals


def verify(prefix, mode):
    files, totals = read_counters(prefix)
    evaluations = totals["substep_tangent_evaluations"]
    perturbed = totals["substep_tangent_perturbed_replays"]
    restored = totals["substep_tangent_restoration_replays"]
    centered = totals["substep_tangent_centered_directions"]
    one_sided = totals["substep_tangent_one_sided_directions"]
    failed = totals["substep_tangent_failed_replays"]
    mismatches = totals["substep_tangent_branch_mismatches"]
    cross_branch = totals["substep_tangent_cross_branch_directions"]
    restoration_mismatches = totals["substep_tangent_restoration_branch_mismatches"]

    if evaluations <= 0:
        raise RuntimeError("the fixture did not construct a multi-substep numerical tangent")
    if perturbed != 12 * evaluations:
        raise RuntimeError(
            f"expected 12 perturbed replays per tangent, got {perturbed}/{evaluations}"
        )
    if restored != evaluations:
        raise RuntimeError(
            f"expected one unperturbed restoration replay per tangent, got {restored}/{evaluations}"
        )
    if centered + one_sided != 6 * evaluations:
        raise RuntimeError(
            "each tangent must produce six directional derivatives: "
            f"centered={centered}, one_sided={one_sided}, evaluations={evaluations}"
        )
    if restoration_mismatches != 0:
        raise RuntimeError(
            "the unperturbed restoration replay did not reproduce the accepted branch history: "
            f"count={restoration_mismatches}"
        )

    if mode == "smooth":
        if failed != 0 or mismatches != 0 or cross_branch != 0 or one_sided != 0:
            raise RuntimeError(
                "smooth tangent fixture was not entirely centered on one branch: "
                f"failed={failed}, mismatches={mismatches}, cross_branch={cross_branch}, "
                f"one_sided={one_sided}"
            )
    elif mode == "transition":
        # This is the production-perturbation qualification case. A constitutive topology
        # transition may occur on the accepted path, but successful perturbed paths should not
        # silently move that transition to another local substep.
        if mismatches != 0 or cross_branch != 0:
            raise RuntimeError(
                "default transition tangent used a successful replay from a different branch "
                f"history: mismatches={mismatches}, cross_branch={cross_branch}"
            )
    elif mode == "probe":
        # The enlarged-perturbation diagnostic must prove that the branch-history detector can
        # observe the cross-branch secant mechanism without changing the production algorithm.
        if mismatches <= 0 or cross_branch <= 0:
            raise RuntimeError(
                "large-perturbation probe did not exercise a successful cross-branch secant: "
                f"mismatches={mismatches}, cross_branch={cross_branch}"
            )

    replay_paths = perturbed + restored
    print(
        "substep tangent diagnostics verified: "
        f"mode={mode}, files={len(files)}, evaluations={evaluations}, "
        f"perturbed_replays={perturbed}, restoration_replays={restored}, "
        f"replay_paths_per_tangent={replay_paths / evaluations:.0f}, "
        f"full_paths_including_accepted_per_tangent={1 + replay_paths / evaluations:.0f}, "
        f"centered={centered}, one_sided={one_sided}, failed={failed}, "
        f"branch_mismatches={mismatches}, cross_branch_directions={cross_branch}"
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=("smooth", "transition", "probe", "clean"))
    parser.add_argument("prefix")
    args = parser.parse_args()

    try:
        if args.mode == "clean":
            clean(args.prefix)
        else:
            verify(args.prefix, args.mode)
    except Exception as error:
        print(f"substep tangent diagnostic check failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
