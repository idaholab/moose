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

import os
import unittest
import subprocess
import MooseDocs

class TestPresentation(unittest.TestCase):
    """
    Test that reveal.js slideshow generation is working.
    """
    html_file = os.path.join('examples', 'presentation', 'index.html')
    working_dir = os.getcwd()

    def clean(self):
        """
        Remove the 'presentation.html' file that is used for testing.
        """
        if os.path.exists(self.html_file):
            os.remove(self.html_file)

    def setUp(self):
        """
        Runs prior to each test.
        """
        os.chdir(os.path.join(MooseDocs.ROOT_DIR, 'docs'))
        self.clean()

    def tearDown(self):
        """
        Runs after each test.
        """
        self.clean()
        os.chdir(self.working_dir)

    def testHTML(self):
        """
        Test that slides are generated
        """
        subprocess.check_output(['./moosedocs.py', 'presentation', 'examples/presentation.md'])
        self.assertTrue(os.path.exists(self.html_file))


if __name__ == '__main__':
    unittest.main(verbosity=2)
