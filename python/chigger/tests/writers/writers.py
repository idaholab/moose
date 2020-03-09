#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import unittest
import chigger

class TestVTKWriters(unittest.TestCase):
    """
    A unittest for the supported output types.
    """
    extensions = ['.png', '.ps', '.tiff', '.bmp', '.jpg']
    basename = 'writers'

    @classmethod
    def setUpClass(cls):
        """
        Clean up old files.
        """
        for ext in cls.extensions:
            filename = cls.basename + ext
            if os.path.exists(filename):
                os.remove(filename)


    def setUp(self):
        """
        Create a window to export.
        """
        file_name = '../input/mug_blocks_out.e'
        self._reader = chigger.exodus.ExodusReader(file_name, adaptive=False)
        self._result = chigger.exodus.ExodusResult(self._reader, cmap='viridis')
        self._window = chigger.RenderWindow(self._result, size=[300,300], style='test')

    def testFormats(self):
        """
        Test that the images are created.
        """
        for ext in self.extensions:
            filename = self.basename + ext
            self._window.write(filename)
            self.assertTrue(os.path.exists(filename))

    def testError(self):
        """
        Test that error message is given with an unknown extension.
        """
        self._window.write('writers.nope')
        output = sys.stdout.getvalue()
        self.assertIn("The filename must end with one of the following", output)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
