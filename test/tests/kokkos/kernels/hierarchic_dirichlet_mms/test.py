import os
import unittest

import mooseutils
import pandas

# Polynomial orders swept, ascending. HIERARCHIC shape functions above first order require the
# second-order geometric element the input file's mesh already uses.
ORDERS = ["SECOND", "THIRD", "FOURTH"]


class TestHierarchicDirichletMMS(unittest.TestCase):
    def test(self):
        executable = os.environ.get(
            "MOOSE_PYTHONUNITTEST_EXECUTABLE"
        ) or mooseutils.find_moose_executable_recursive(os.getcwd())

        errors = []

        for order in ORDERS:
            file_base = "hierarchic_dirichlet_mms_{}".format(order.lower())

            mooseutils.run_executable(
                executable,
                "-i",
                "hierarchic_dirichlet_mms.i",
                "Variables/u/order={}".format(order),
                "Outputs/file_base={}".format(file_base),
                suppress_output=True,
            )

            data = pandas.read_csv("{}.csv".format(file_base))
            errors.append(data["l2_error"].iloc[-1])

        for i in range(1, len(errors)):
            self.assertLess(
                errors[i],
                errors[i - 1],
                "l2_error did not strictly decrease from order {} ({}) to order {} ({})".format(
                    ORDERS[i - 1], errors[i - 1], ORDERS[i], errors[i]
                ),
            )

        ratio = errors[0] / errors[-1]
        self.assertGreaterEqual(
            ratio,
            5.0,
            "l2_error only dropped by a factor of {} from order {} to order {}, expected at "
            "least 5x".format(ratio, ORDERS[0], ORDERS[-1]),
        )


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
