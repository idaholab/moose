#!/usr/bin/env python
#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################

import os
import unittest
import subprocess
import shutil
import tempfile
import mooseutils
import MooseDocs

class TestSQA(unittest.TestCase):
    """
    Test the build works across directories.
    """
    SITE_DIR = tempfile.mkdtemp(dir=os.path.join(os.getenv('HOME'), '.local', 'share', 'moose'))
    WORKING_DIR = os.getcwd()

    def setUp(self):
        """
        Runs prior to each test.
        """
        os.chdir(os.path.join(MooseDocs.ROOT_DIR, 'test', 'docs'))
        if not os.path.exists(self.SITE_DIR):
            os.makedirs(self.SITE_DIR)

    def tearDown(self):
        """
        Runs after each test.
        """
        os.chdir(self.WORKING_DIR)
        shutil.rmtree(self.SITE_DIR)

    def testBuild(self):
        """
        Test that sqa demo is working.
        """
        exe = mooseutils.find_moose_executable(os.path.join(MooseDocs.MOOSE_DIR, 'test'))
        self.assertTrue(os.path.isfile(exe), "The moose_test executable does not exist.")

        c = ['./moosedocs.py', 'build', '--config-file', 'sqa.yml', '--clean', '--site-dir',
             self.SITE_DIR]
        proc = subprocess.Popen(c, cwd=os.path.join(MooseDocs.MOOSE_DIR, 'test', 'docs'),
                                stdout=subprocess.PIPE)
        out = proc.stdout.read()
        self.assertIn(out, 'WARNINGS: 0  ERRORS: 4\n')

        with open(os.path.join(self.SITE_DIR, 'sqa', 'test_srs', 'index.html'), 'r') as fid:
            html = fid.read()
        self.assertIn('Testing testing testing', html)
        self.assertIn('Missing Template Item: project_description', html)
        self.assertIn('Missing Template Item: system_scope', html)
        self.assertIn('<span class="moose-collection-name">F1.50</span>', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Transient Analysis</div>',
                      html)
        self.assertIn('<li id="requirement-F1.10">', html)

        with open(os.path.join(self.SITE_DIR, 'sqa', 'test_rtm', 'index.html'), 'r') as fid:
            html = fid.read()
        self.assertIn('<div class="collapsible-header moose-group-header">Transient Analysis</div>',
                      html)
        self.assertIn('<a href="../test_srs/index.html">F1.10</a>', html)
        self.assertIn('<span class="moose-sqa-error">F9.99</span>', html)

        with open(os.path.join(self.SITE_DIR, 'sqa', 'test_v_and_v', 'index.html'), 'r') as fid:
            html = fid.read()
        self.assertIn('<a href="validation/V1-01/index.html">V1.01</a>', html)
        link = os.path.join(MooseDocs.ROOT_DIR,
                            'test/docs/content/sqa/test_v_and_v/validation/V1-02.md')
        self.assertIn('<a class="moose-bad-link" href="{}">V1.02</a>'.format(link), html)
        self.assertIn('<span class="new badge danger" data-badge-caption="danger">1', html)
        self.assertIn('<span class="new badge error" data-badge-caption="error">1', html)

        with open(os.path.join(self.SITE_DIR, 'sqa', 'index.html'), 'r') as fid:
            html = fid.read()
        self.assertIn('<span class="moose-page-status" data-filename="test_srs/index.html">', html)
        self.assertIn('<span class="new badge pass" data-badge-caption="pass">', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
