#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import types
import unittest

from MooseDocs import common

class TestCheckType(unittest.TestCase):
    def testCallable(self):
        func = lambda: True
        common.check_type('foo', func, types.FunctionType)

        with self.assertRaises(Exception) as e:
            common.check_type('foo', 42, types.FunctionType)
        self.assertEqual("The argument 'foo' must be callable but <type 'int'> was provided.",
                         e.exception.message)

        with self.assertRaises(Exception) as e:
            common.check_type('foo', 42, list)

        gold = "The argument 'foo' must be of type <type 'list'> but <type 'int'> was provided."
        self.assertIn(gold, e.exception.message)

if __name__ == '__main__':
    unittest.main(verbosity=2)
