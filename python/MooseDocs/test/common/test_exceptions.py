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
from MooseDocs.common import exceptions

class TestExceptions(unittest.TestCase):
    def testMooseDocsException(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            raise exceptions.MooseDocsException("{}", 'foo')
        self.assertEqual('foo', str(e.exception))

if __name__ == '__main__':
    unittest.main(verbosity=2)
