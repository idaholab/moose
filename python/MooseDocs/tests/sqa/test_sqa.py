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
#pylint: enable=missing-docstring

import os
import unittest
import subprocess
import shutil
import tempfile
import anytree
import mooseutils
import MooseDocs
from MooseDocs import common
from MooseDocs import testing

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
        self.assertIn(out, 'WARNINGS: 0  ERRORS: 2\n')

        with open(os.path.join(self.SITE_DIR, 'sqa', 'srs', 'index.html'), 'r') as fid:
            html = fid.read()
        self.assertIn('Testing testing testing', html)
        self.assertIn('Missing Template Item: project_description', html)
        self.assertIn('Missing Template Item: system_scope', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
