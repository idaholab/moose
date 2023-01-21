#!/usr/bin/env python3
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
import glob
import moosesqa
import mooseutils

class TestSQA(unittest.TestCase):
    """Tests that make sure that SQA templates and SQA collections are documented."""

    @classmethod
    def setUpClass(cls):
        cls.ROOT_DIR = mooseutils.git_root_dir(os.path.dirname(__file__))
        cls.TEMPLATE_DIR = os.path.join(cls.ROOT_DIR, 'framework', 'doc', 'content', 'templates', 'sqa')
        cls.TEMPLATE_NAMES =['far.md.template', 'rtm.md.template', 'sdd.md.template', 'srs.md.template',
                             'stp.md.template', 'vvr.md.template', 'cci.md.template', 'scs.md.template',
                             'sll.md.template', 'app_index.md.template',
                             'app_far.md.template', 'app_rtm.md.template', 'app_sdd.md.template',
                             'app_srs.md.template', 'app_stp.md.template', 'app_vvr.md.template',
                             'app_cci.md.template', 'app_scs.md.template', 'app_sll.md.template',
                             'module_far.md.template', 'module_rtm.md.template', 'module_sdd.md.template',
                             'module_srs.md.template', 'module_stp.md.template', 'module_vvr.md.template',
                             'module_sll.md.template']
        cls.DOC_FILE = os.path.join(cls.ROOT_DIR, 'python', 'doc', 'content', 'python', 'MooseDocs', 'extensions', 'sqa.md')
        cls.COLLECTIONS = {'FUNCTIONAL', 'USABILITY', 'PERFORMANCE', 'SYSTEM', 'FAILURE_ANALYSIS'}

    def testTemplatesFiles(self):
        for tname in TestSQA.TEMPLATE_NAMES:
            fname = os.path.join(TestSQA.TEMPLATE_DIR, tname)
            self.assertTrue(os.path.isfile(fname))

        for filename in glob.glob(os.path.join(TestSQA.TEMPLATE_DIR, '*.md.template')):
            self.assertIn(os.path.basename(filename), TestSQA.TEMPLATE_NAMES)


    def testTemplateDocs(self):
        self.assertTrue(os.path.isfile(TestSQA.DOC_FILE))
        with open(TestSQA.DOC_FILE, 'r') as fid:
            content = fid.read()

        for tname in TestSQA.TEMPLATE_NAMES:
            self.assertIn('+{}+\\'.format(tname), content)

    def testCollectionNames(self):
        self.assertEqual(moosesqa.MOOSESQA_COLLECTIONS, TestSQA.COLLECTIONS)

    def testCollectionDocs(self):
        self.assertTrue(os.path.isfile(TestSQA.DOC_FILE))
        with open(TestSQA.DOC_FILE, 'r') as fid:
            content = fid.read()

        for cname in TestSQA.COLLECTIONS:
            self.assertIn('+{}+:'.format(cname), content)

if __name__ == '__main__':
    unittest.main(verbosity=2)
