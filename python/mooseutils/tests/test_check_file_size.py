#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from mooseutils import check_file_size

class TestCheckFileSize(unittest.TestCase):
    """
    Test that the size function returns something.
    """

    def testBasic(self):
        results = check_file_size(size=1) # No files greater than 1Mb
        self.assertEqual(results, [])
        results = check_file_size(size=0)
        self.assertNotEqual(results, [])

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
