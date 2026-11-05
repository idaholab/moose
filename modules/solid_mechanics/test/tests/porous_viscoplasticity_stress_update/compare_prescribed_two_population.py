#!/usr/bin/env python3
import csv
import math
import sys

if len(sys.argv) != 3:
    raise SystemExit(f"usage: {sys.argv[0]} reference.csv candidate.csv")

reference_path, candidate_path = sys.argv[1:]
columns = (
    "time",
    "disp_x",
    "disp_y",
    "avg_hydro",
    "avg_vonmises",
    "eff_creep_strain",
    "porosity",
    "population_0_porosity",
    "population_1_porosity",
    "population_0_effective_hydrostatic_stress",
    "population_1_effective_hydrostatic_stress",
)


def read(path):
    with open(path, newline="") as stream:
        rows = list(csv.DictReader(stream))
    if not rows:
        raise SystemExit(f"{path}: no CSV rows")
    return rows


reference = read(reference_path)
candidate = read(candidate_path)
if len(reference) != len(candidate):
    raise SystemExit(
        f"row-count mismatch: {reference_path} has {len(reference)}, "
        f"{candidate_path} has {len(candidate)}"
    )

for row_index, (ref_row, test_row) in enumerate(zip(reference, candidate)):
    for column in columns:
        if column not in ref_row or column not in test_row:
            raise SystemExit(f"missing comparison column {column!r}")
        ref = float(ref_row[column])
        test = float(test_row[column])
        if not math.isclose(ref, test, rel_tol=2.0e-7, abs_tol=1.0e-11):
            raise SystemExit(
                f"{column} mismatch at row {row_index}: reference={ref:.16e}, "
                f"candidate={test:.16e}"
            )

print(f"prescribed two-population AD/non-AD comparison passed for {len(reference)} rows")
