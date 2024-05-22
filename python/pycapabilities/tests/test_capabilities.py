#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest

class TestCapabilities(unittest.TestCase):
    def test(self):

        # Load the packages
        import pycapabilities

        # Dummy capabilities
        cap = {
            'petsc': ['3.9.1', "PETSc doc"],
            'party': [True, "Party partitioner available"],
            'adsize': [50, "Derivative storage size"],
            'compiler': ['GCC', "Compiler used to build moose"]
        }

        self.assertEqual(pycapabilities.check("!petsc", cap)[0], pycapabilities.CERTAIN_FAIL)
        self.assertEqual(pycapabilities.check("!unknown", cap)[0], pycapabilities.POSSIBLE_PASS)
        self.assertEqual(pycapabilities.check("adsize > 20", cap)[0], pycapabilities.CERTAIN_PASS)
        self.assertEqual(pycapabilities.check("!party", cap)[0], pycapabilities.CERTAIN_FAIL)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
