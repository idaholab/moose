#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from mooseutils import camel_to_space

class TestCamelCaseToSpace(unittest.TestCase):
    """
    Test that the size function returns something.
    """
    def assertInvert(self, text, gold):
        self.assertEqual(camel_to_space(text), gold)

    def testBasic(self):
        self.assertInvert('ThisIsSomethingLong', 'This Is Something Long')
        self.assertInvert('EBSD', 'EBSD')
        self.assertInvert('ICs', 'ICs')
        self.assertInvert('lowerUpper', 'lower Upper')
        self.assertInvert('lowerXYZupper', 'lower XYZupper')
        self.assertInvert('UpperXYZ', 'Upper XYZ')


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
