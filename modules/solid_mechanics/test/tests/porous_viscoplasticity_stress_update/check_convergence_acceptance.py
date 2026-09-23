#!/usr/bin/env python3

import re

_ORDINARY_TOL = 1.0e-12
_STAGNATION_TOL = 0.5
_PATTERN = re.compile(
    r"analytical local solve: .*?"
    r"ordinary_convergence_norm = ([0-9.eE+-]+) "
    r"stagnation_convergence_norm = ([0-9.eE+-]+)"
)


def custom_evaluation(output):
    if "Reduced porosity" in output:
        return False

    for ordinary_text, stagnation_text in _PATTERN.findall(output):
        ordinary = float(ordinary_text)
        stagnation = float(stagnation_text)
        if ordinary > 10.0 * _ORDINARY_TOL and stagnation <= _STAGNATION_TOL:
            return True

    return False
