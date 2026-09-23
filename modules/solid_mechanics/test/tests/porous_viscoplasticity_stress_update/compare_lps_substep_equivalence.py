#!/usr/bin/env python3

import csv
import math
import sys

FIELDS = (
    "avg_hydro",
    "avg_vonmises",
    "gauge_stress",
    "eff_creep_strain",
    "porosity",
)

TOLERANCES = {
    "avg_hydro": (5.0e-3, 1.0e-1),
    "avg_vonmises": (5.0e-3, 1.0e-4),
    "gauge_stress": (5.0e-3, 1.0e-4),
    "eff_creep_strain": (5.0e-3, 1.0e-10),
    # Porosity is the local constitutive state itself, so require much tighter agreement than the
    # historical 0.5% CSVDiff used by the old long substepping test.
    "porosity": (5.0e-5, 1.0e-10),
}


def final_row(path):
    with open(path, newline="") as stream:
        rows = list(csv.DictReader(stream))
    if not rows:
        raise RuntimeError(f"{path} contains no data rows")
    return rows[-1]


def main():
    if len(sys.argv) != 3:
        raise SystemExit(
            "usage: compare_lps_substep_equivalence.py REFERENCE_CSV SUBSTEPPED_CSV"
        )

    reference = final_row(sys.argv[1])
    candidate = final_row(sys.argv[2])

    reference_time = float(reference["time"])
    candidate_time = float(candidate["time"])
    if not math.isclose(reference_time, 1.0, rel_tol=0.0, abs_tol=1.0e-12):
        raise SystemExit(f"reference terminal time is {reference_time:.17g}, expected 1")
    if not math.isclose(candidate_time, 1.0, rel_tol=0.0, abs_tol=1.0e-12):
        raise SystemExit(f"substepped terminal time is {candidate_time:.17g}, expected 1")

    mismatches = []
    for field in FIELDS:
        ref = float(reference[field])
        test = float(candidate[field])
        if not math.isfinite(ref) or not math.isfinite(test):
            mismatches.append(f"{field}: nonfinite reference={ref} candidate={test}")
            continue
        rel_tol, abs_tol = TOLERANCES[field]
        if not math.isclose(test, ref, rel_tol=rel_tol, abs_tol=abs_tol):
            mismatches.append(
                f"{field}: reference={ref:.17g} candidate={test:.17g} "
                f"rel_tol={rel_tol:g} abs_tol={abs_tol:g}"
            )

    if mismatches:
        raise SystemExit(
            "local-substep/fine-global equivalence failed:\n  " + "\n  ".join(mismatches)
        )

    print(
        "porous-LPS local-substep/fine-global equivalence passed for "
        f"{len(FIELDS)} final-state quantities"
    )


if __name__ == "__main__":
    main()
