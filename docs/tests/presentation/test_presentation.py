#!/usr/bin/env python
import os
import unittest
import subprocess
import MooseDocs

class TestPresentation(unittest.TestCase):
    """
    Test that reveal.js slideshow generation is working.
    """
    html_file = os.path.join('examples', 'presentation.html')
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

    def testPDF(self):
        """
        Test that slides are generated
        """
        out = subprocess.check_output(['./moosedocs.py', 'presentation' , 'examples/presentation.md'])
        self.assertTrue(os.path.exists(self.html_file))
