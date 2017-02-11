#!/usr/bin/env python
import os
import unittest
import subprocess
import MooseDocs

class TestGeneration(unittest.TestCase):
    """
    Test that pdf and tex is generated from latex command.
    """
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
        out = subprocess.check_output(['./moosedocs.py', 'latex' , 'examples/report.md'])
        self.assertTrue(os.path.exists(self.pdf_file))

    def testTex(self):
        """
        Test that the tex file is generated.
        """
        out = subprocess.check_output(['./moosedocs.py', 'latex' , 'examples/report.md', '--output', self.tex_file])
        self.assertTrue(os.path.exists(self.tex_file))

if __name__ == '__main__':
    unittest.main(verbosity=2)
