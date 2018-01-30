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

class TestGeneration(unittest.TestCase):
    pdf_file = os.path.join('examples', 'report.pdf')
    tex_file = os.path.join('examples', 'report.tex')
    working_dir = os.getcwd()

    def clean(self):
        """
        Remove the 'report.pdf' file that is used for testing.
        """
        for filename in [self.pdf_file, self.tex_file]:
            if os.path.exists(filename):
                os.remove(filename)

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

    def testPDF(self):
        """
        Test that the PDF is generated.
        """
        subprocess.check_output(['./moosedocs.py', 'latex', 'examples/report.md'])
        self.assertTrue(os.path.exists(self.pdf_file))

    def testTex(self):
        """
        Test that the tex file is generated.
        """
        subprocess.check_output(['./moosedocs.py', 'latex', 'examples/report.md',
                                 '--output', self.tex_file])
        self.assertTrue(os.path.exists(self.tex_file))

if __name__ == '__main__':
    unittest.main(verbosity=2)
