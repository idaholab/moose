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

class TestBuild(unittest.TestCase):
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

    @staticmethod
    def finder(root, name):
        """Helper for finding tree items"""
        filter_ = lambda n: n.full_name.endswith(name)
        return [node for node in anytree.iterators.PreOrderIter(root, filter_=filter_)]

    def testTree(self):
        """
        Test the file tree creation.
        """
        config = dict()
        config['framework'] = dict(base='docs/content',
                                   include=['docs/content/documentation/systems/' \
                                            'Functions/**'])
        config['test'] = dict(base='test/docs/content', include=['test/docs/content/**'])
        node = common.moose_docs_file_tree(config)

        func = self.finder(node, 'PostprocessorFunction')[0]
        self.assertTrue(func)

        self.assertEqual(func.filename,
                         os.path.join(MooseDocs.ROOT_DIR, 'test', 'docs', 'content',
                                      'documentation', 'systems', 'Functions', 'moose_test',
                                      'PostprocessorFunction.md'))
        self.assertEqual(func.destination, 'documentation/systems/Functions/moose_test/' \
                                           'PostprocessorFunction/index.html')

    def testBuild(self):
        """
        Test that build command is working.
        """
        exe = mooseutils.find_moose_executable(os.path.join(MooseDocs.MOOSE_DIR, 'test'))
        self.assertTrue(os.path.isfile(exe), "The moose_test executable does not exist.")

        try:
            c = ['./moosedocs.py', 'build', '--clean', '--site-dir', self.SITE_DIR]
            out = subprocess.check_output(c, cwd=os.path.join(MooseDocs.MOOSE_DIR, 'test', 'docs'))
        except subprocess.CalledProcessError:
            self.assertFalse(True, #pytlint: disable=W1503
                             "The './moosedocs.py build' command returned a non-zero value.")
        self.assertEqual(out, 'WARNINGS: 0  ERRORS: 0\n')

        root = os.path.join(self.SITE_DIR, 'documentation', 'systems')
        self.assertTrue(os.path.isfile(os.path.join(root, 'Functions', 'framework',
                                                    'ParsedGradFunction', 'index.html')))
        self.assertFalse(os.path.isfile(os.path.join(root, 'Functions', 'framework',
                                                     'ParsedFunction', 'index.html')))
        self.assertTrue(os.path.isfile(os.path.join(root, 'Adaptivity', 'Markers', 'framework',
                                                    'BoxMarker', 'index.html')))
        self.assertFalse(os.path.isfile(os.path.join(root, 'Adaptivity', 'Markers', 'framework',
                                                     'ComboMarker', 'index.html')))
        self.assertTrue(os.path.isfile(os.path.join(root, 'Functions', 'moose_test',
                                       'PostprocessorFunction', 'index.html')))


if __name__ == '__main__':
    unittest.main(verbosity=2)
