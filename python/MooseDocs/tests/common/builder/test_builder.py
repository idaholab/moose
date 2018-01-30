#!/usr/bin/env python
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import mooseutils

from MooseDocs.common import Builder
from MooseDocs.testing import LogTestCase

class TestBuilder(LogTestCase):
    """
    Tests Builder base class
    """
    def testErrors(self):
        builder = Builder(None, None)
        with self.assertRaises(mooseutils.MooseException) as e:
            list(builder)
        self.assertIn("The 'init' method must be called prior to iterating", str(e.exception))

        with self.assertRaises(mooseutils.MooseException) as e:
            builder.count()
        self.assertIn("The 'init' method must be called prior to count", str(e.exception))

        with self.assertRaises(mooseutils.MooseException) as e:
            builder.build()
        self.assertIn("The 'init' method must be called prior to build", str(e.exception))

        with self.assertRaises(NotImplementedError) as e:
            builder.buildNodes()
        self.assertIn("The 'buildNodes' method must be defined", str(e.exception))

if __name__ == '__main__':
    unittest.main(verbosity=2)
