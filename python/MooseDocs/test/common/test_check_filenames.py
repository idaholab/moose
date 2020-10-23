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
import MooseDocs
from MooseDocs.common import check_filenames, exceptions


class TestProjectFind(unittest.TestCase):
    def test(self):
        """
        Test that class with h and C files are located.
        """
        MooseDocs.PROJECT_FILES = ['file0.md', '/path/to/another/file0.md', 'image1.png']

        self.assertEqual(check_filenames('image1.png'), 'image1.png')
        self.assertEqual(check_filenames('another/file0.md'), '/path/to/another/file0.md')

        with self.assertRaises(exceptions.MooseDocsException) as e:
            self.assertEqual(check_filenames('file0.md'), ['file0.md', '/path/to/another/file0.md'])
        self.assertIn('Multiple files', str(e.exception))
        self.assertIn('another/file0.md', str(e.exception))

        with self.assertRaises(exceptions.MooseDocsException) as e:
            self.assertEqual(check_filenames('wrong'), [])
        self.assertIn('does not exist in the repository', str(e.exception))
        self.assertIn('git ls-files', str(e.exception))


if __name__ == '__main__':
    unittest.main(verbosity=2)
