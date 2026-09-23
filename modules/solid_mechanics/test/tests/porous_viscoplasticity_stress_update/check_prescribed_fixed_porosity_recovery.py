#!/usr/bin/env python3

import csv
import glob
import math
import os
import sys


START_TIME = 1.0
END_TIME = 1.1
NOMINAL_DT = 0.1
TIME_TOL = 1.0e-14
DT_TOL = 1.0e-14
FLOOR = 0.05
FLOOR_TOL = 1.0e-9

STATE_TOLERANCES = {
    "porosity": (5.0e-8, 5.0e-10),
    "population_0_porosity": (5.0e-8, 5.0e-10),
    "population_1_porosity": (5.0e-8, 5.0e-10),
    "population_0_effective_hydrostatic_stress": (5.0e-8, 10.0),
    "population_1_effective_hydrostatic_stress": (5.0e-8, 10.0),
    "avg_hydro": (5.0e-8, 10.0),
    "avg_vonmises": (5.0e-8, 10.0),
    "eff_creep_strain": (5.0e-8, 5.0e-10),
    "disp_x": (5.0e-8, 5.0e-12),
    "disp_y": (5.0e-8, 5.0e-12),
}


def fail(message):
    raise SystemExit(f"generic fixed-porosity recovery check failed: {message}")


def counter_paths(prefix):
    return sorted(glob.glob(f"{prefix}_*_rank*_thread*_instance*.csv"))


def clean_counter_files(prefix):
    paths = counter_paths(prefix)
    for path in paths:
        os.remove(path)
    print(f"removed {len(paths)} stale performance-counter file(s) for {prefix!r}")


def read_counter_files(prefix):
    paths = counter_paths(prefix)
    if not paths:
        fail(f"no performance-counter files matched {prefix!r}")

    counters = {}
    for path in paths:
        with open(path, newline="") as stream:
            for row in csv.reader(stream):
                if len(row) != 2 or row[0] in {"key", "object", "rank", "thread", "is_ad"}:
                    continue
                try:
                    value = int(row[1])
                except ValueError:
                    continue
                counters[row[0]] = counters.get(row[0], 0) + value
    return paths, counters


def read_rows(path):
    with open(path, newline="") as stream:
        rows = list(csv.DictReader(stream))
    if not rows:
        fail(f"{path!r} contains no CSV data rows")
    if "time" not in rows[-1]:
        fail(f"{path!r} does not contain a time column")
    return rows


def as_float(row, key, path):
    if key not in row or row[key] == "":
        fail(f"{path!r} does not contain a populated {key!r} column")
    try:
        value = float(row[key])
    except ValueError as error:
        fail(f"{path!r} has a nonnumeric {key!r} value {row[key]!r}: {error}")
    if not math.isfinite(value):
        fail(f"{path!r} has a nonfinite {key!r} value {value}")
    return value


def verify_continuation_grid(path):
    rows = read_rows(path)
    times = [as_float(row, "time", path) for row in rows]

    if not any(math.isclose(time, START_TIME, rel_tol=0.0, abs_tol=TIME_TOL) for time in times):
        fail(f"{path!r} does not contain the checkpoint time t={START_TIME}")

    continuation_times = [time for time in times if time > START_TIME + TIME_TOL]
    if len(continuation_times) != 1 or not math.isclose(
        continuation_times[0], END_TIME, rel_tol=0.0, abs_tol=TIME_TOL
    ):
        fail(
            f"expected exactly one accepted continuation step {START_TIME} -> {END_TIME}; "
            f"accepted continuation times are {continuation_times}"
        )

    final_row = rows[-1]
    final_time = as_float(final_row, "time", path)
    accepted_dt = as_float(final_row, "accepted_dt", path)
    if not math.isclose(final_time, END_TIME, rel_tol=0.0, abs_tol=TIME_TOL):
        fail(f"the recovery continuation ended at t={final_time:.17g}, expected {END_TIME}")
    if not math.isclose(accepted_dt, NOMINAL_DT, rel_tol=0.0, abs_tol=DT_TOL):
        fail(
            f"the accepted continuation timestep was dt={accepted_dt:.17g}, "
            f"expected the uncut nominal dt={NOMINAL_DT}"
        )

    for field in ("population_0_porosity", "population_1_porosity"):
        value = as_float(final_row, field, path)
        if not math.isclose(value, FLOOR, rel_tol=0.0, abs_tol=FLOOR_TOL):
            fail(f"{field}={value:.17g} did not remain on the active floor {FLOOR}")

    creep = as_float(final_row, "eff_creep_strain", path)
    if abs(creep) < 1.0e-10:
        fail("the continuation did not retain a nonzero deviatoric viscoplastic response")

    return rows, final_row, accepted_dt


def verify_recovery(prefix, output_csv):
    paths, counters = read_counter_files(prefix)
    solves = counters.get("independent_fixed_porosity_mechanical_solves", 0)
    iterations = counters.get("independent_fixed_porosity_mechanical_iterations", 0)
    successes = counters.get("independent_fixed_porosity_recovery_successes", 0)

    if solves < 1:
        fail("the independent fixed-porosity mechanical recovery was never attempted")
    if successes < 1:
        fail("no independent fixed-porosity recovery was accepted")
    if successes > solves:
        fail(f"accepted recoveries ({successes}) exceed recovery attempts ({solves})")
    if iterations < successes:
        fail(
            f"recovery iterations ({iterations}) are inconsistent with accepted recoveries ({successes})"
        )

    _, _, accepted_dt = verify_continuation_grid(output_csv)
    print(
        "generic independent fixed-porosity recovery verified: "
        f"files={len(paths)}, attempts={solves}, iterations={iterations}, "
        f"successes={successes}, accepted_dt={accepted_dt:.17g}, final_time={END_TIME:.17g}"
    )


def compare_final_states(recovered_csv, reference_csv):
    _, recovered, recovered_dt = verify_continuation_grid(recovered_csv)
    _, reference, reference_dt = verify_continuation_grid(reference_csv)

    if not math.isclose(recovered_dt, reference_dt, rel_tol=0.0, abs_tol=DT_TOL):
        fail(
            f"recovered/reference accepted dt mismatch: {recovered_dt:.17g} vs {reference_dt:.17g}"
        )

    mismatches = []
    for field, (rel_tol, abs_tol) in STATE_TOLERANCES.items():
        recovered_value = as_float(recovered, field, recovered_csv)
        reference_value = as_float(reference, field, reference_csv)
        if not math.isclose(recovered_value, reference_value, rel_tol=rel_tol, abs_tol=abs_tol):
            mismatches.append(
                f"{field}: recovered={recovered_value:.17g}, reference={reference_value:.17g}, "
                f"rel_tol={rel_tol:g}, abs_tol={abs_tol:g}"
            )

    if mismatches:
        fail(
            "recovered terminal state differs from normally converged reference:\n  "
            + "\n  ".join(mismatches)
        )

    print(
        "generic recovered state matches the normally converged reference "
        f"for {len(STATE_TOLERANCES)} mechanics/pore quantities at t={END_TIME:.17g}"
    )


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "--clean":
        clean_counter_files(sys.argv[2])
        return

    if len(sys.argv) == 4 and sys.argv[1] == "--compare":
        compare_final_states(sys.argv[2], sys.argv[3])
        return

    if len(sys.argv) == 3:
        verify_recovery(sys.argv[1], sys.argv[2])
        return

    fail(
        "usage: check_prescribed_fixed_porosity_recovery.py PERF_PREFIX OUTPUT_CSV\n"
        "   or: check_prescribed_fixed_porosity_recovery.py --clean PERF_PREFIX\n"
        "   or: check_prescribed_fixed_porosity_recovery.py --compare RECOVERED_CSV REFERENCE_CSV"
    )


if __name__ == "__main__":
    main()
