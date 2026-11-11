#!/usr/bin/env python3

import math
import re

_PATTERN = re.compile(
    r"adaptive history substep predictor at element \d+ _qp=0 .*?"
    r"previous_accepted_rate=([0-9.eE+-]+) "
    r"global_dt=([0-9.eE+-]+) .*?"
    r"predicted_increment=([0-9.eE+-]+)"
)


def custom_evaluation(output):
    if "Intentional adaptive-history rollback test cutback." not in output:
        return False

    samples = [tuple(map(float, match)) for match in _PATTERN.findall(output)]
    for i, (failed_rate, failed_dt, failed_prediction) in enumerate(samples):
        if failed_rate <= 0.0 or failed_prediction <= 0.0:
            continue
        if not math.isclose(failed_dt, 2.0, rel_tol=1.0e-12, abs_tol=1.0e-12):
            continue

        for retry_rate, retry_dt, retry_prediction in samples[i + 1 :]:
            if not math.isclose(retry_dt, 1.0, rel_tol=1.0e-12, abs_tol=1.0e-12):
                continue

            same_history = math.isclose(
                retry_rate, failed_rate, rel_tol=1.0e-12, abs_tol=1.0e-16
            )
            scaled_prediction = math.isclose(
                failed_prediction,
                retry_prediction * failed_dt / retry_dt,
                rel_tol=5.0e-6,
                abs_tol=1.0e-14,
            )
            return same_history and scaled_prediction

    return False
