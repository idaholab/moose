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
import unittest
import mock
import logging
import moosesqa
import mooseutils

logging.basicConfig()

@unittest.skipIf(mooseutils.git_version() < (2,11,4), "Git version must at least 2.11.4")
class TestCheckRequirements(unittest.TestCase):

    def setUp(self):
        self._patcher = mock.patch('mooseutils.colorText', side_effect=lambda t, c, **kwargs: t)
        self._patcher.start()

    def testDeprecated(self):
        req0 = moosesqa.Requirement(name='req0', deprecated=True,
                                    requirement='requirement', requirement_line=0,
                                    design=['Diffusion.md'], design_line=1,
                                    issues=['issues'], issues_line=2,
                                    verification=['Diffusion.md'], verification_line=3,
                                    validation=['Diffusion.md'], validation_line=4)
        req0.detail = 'text'
        req0.detail_line = 5

        detail0 = moosesqa.Detail(name='req0-0')
        req0.details = [detail0]

        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Deprecated test with 'requirement'", cm.output[0])
        self.assertIn(":0", cm.output[0])

        self.assertIn("Deprecated test with 'design'", cm.output[1])
        self.assertIn(":1", cm.output[1])

        self.assertIn("Deprecated test with 'issues'", cm.output[2])
        self.assertIn(":2", cm.output[2])

        self.assertIn("Deprecated test with 'verification'", cm.output[3])
        self.assertIn(":3", cm.output[3])

        self.assertIn("Deprecated test with 'validation'", cm.output[4])
        self.assertIn(":4", cm.output[4])

        self.assertIn("Deprecated test with 'detail'", cm.output[5])
        self.assertIn(":5", cm.output[5])

        self.assertIn("Deprecated test with sub-block(s)", cm.output[6])

    def testDuplicates(self):
        req0 = moosesqa.Requirement(name='req0', requirement='requirement', design=['Diffusion.md'], issues=['#1234'])
        req0.specification = moosesqa.TestSpecification()
        req1 = moosesqa.Requirement(name='req1', requirement='requirement', design=['Diffusion.md'], issues=['#1234'])
        req1.specification = moosesqa.TestSpecification()

        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0, req1])
        self.assertIn('Duplicate requirements found', cm.output[0])

        with self.assertLogs(level='WARNING') as cm:
            moosesqa.check_requirements([req0, req1], duplicate_requirement=logging.WARNING)
        self.assertIn('Duplicate requirements found', cm.output[0])

    def testDuplicateDetails(self):
        det0 = moosesqa.Detail(name='det0', detail='detail')
        det1 = moosesqa.Detail(name='det1', detail='detail')
        req0 = moosesqa.Requirement(name='req0', requirement='requirement', design=['Diffusion.md'], issues=['#1234'], details=[det0, det1])
        with self.assertLogs(level='WARNING') as cm:
            moosesqa.check_requirements([req0], duplicate_detail=logging.WARNING)
        self.assertIn('Duplicate details found', cm.output[0])

    def testMissing(self):
        req0 = moosesqa.Requirement(name='req0', requirement='requirement', design=['Diffusion.md'], issues=['#1234'])
        req0.specification = moosesqa.TestSpecification()
        req1 = moosesqa.Requirement(name='req1')
        req1.specification = moosesqa.TestSpecification()

        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0, req1])
        self.assertIn("No 'requirement', 'design', and 'issues' supplied", cm.output[0])

        req1.design = ['something']
        req1.design_line = 2
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0, req1])
        self.assertIn("No 'requirement' supplied", cm.output[0])
        self.assertIn("No 'issues' supplied", cm.output[1])

        req1.requirement = 'some requirement'
        req1.issues = ['something']
        req1.design = None
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0, req1])
        self.assertIn("No 'design' supplied", cm.output[0])

    def testEmpty(self):
        req0 = moosesqa.Requirement(name='req0',
                                    requirement='', requirement_line=0,
                                    design=[], design_line=1,
                                    issues=[], issues_line=2)

        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Empty 'requirement' supplied", cm.output[0])
        self.assertIn(":0", cm.output[0])

        self.assertIn("Empty 'design' supplied", cm.output[1])
        self.assertIn(":1", cm.output[1])

        self.assertIn("Empty 'issues' supplied", cm.output[2])
        self.assertIn(":2", cm.output[2])


    def testVerificationValidation(self):
        req0 = moosesqa.Requirement(requirement='requirement', design=['Diffusion.md'], issues=['#1234'],
                                    verification=[], verification_line=1,
                                    validation=[], validation_line=2)

        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Empty 'verification' supplied", cm.output[0])
        self.assertIn(":1", cm.output[0])
        self.assertIn("Empty 'validation' supplied", cm.output[1])
        self.assertIn(":2", cm.output[1])

    def testDetail(self):
        # Top-level detail
        req0 = moosesqa.Requirement(name='req0', requirement='requirement', design=['Diffusion.md'], issues=['#1234'])
        req0.detail = 'wrong'
        req0.detail_line = 1
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Top level 'detail' supplied", cm.output[0])
        self.assertIn(":1", cm.output[0])

        # Missing/empty detail
        req0 = moosesqa.Requirement(name='req0', requirement='requirement', design=['Diffusion.md'], issues=['#1234'])
        detail0 = moosesqa.Detail(name='req0-0')
        req0.details = [detail0]

        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("No 'detail' supplied", cm.output[0])

        detail0.detail = ''
        detail0.detail_line = 1
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Empty 'detail' supplied", cm.output[0])
        self.assertIn(":1", cm.output[0])

        # Extra in sub-block
        detail0.detail = 'detail'
        detail0.requirement = 'text'
        detail0.requirement_line = 1
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Extra 'requirement' supplied", cm.output[0])
        self.assertIn(":1", cm.output[0])

        detail0.requirement = None
        detail0.design = ['text']
        detail0.design_line = 1
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Extra 'design' supplied", cm.output[0])
        self.assertIn(":1", cm.output[0])

        detail0.design = None
        detail0.issues = ['#1234']
        detail0.issues_line = 1
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Extra 'issues' supplied", cm.output[0])
        self.assertIn(":1", cm.output[0])

    def testIssuesFormat(self):
        issues = ['11600c68be9e8f77e4870d7b8efb26e5ada10a5a',
                  '11600c68be9e8f77e4870d7b8efb26e5ada10a5a23423', # error, too long
                  '11600', # error, too short or no #
                  '26exada10a5a', # error, invalid hex
                  '#12345',
                  '#1234c',# # error, invalid hex
                  'app#1234',
                  'app-7#1234']
        req0 = moosesqa.Requirement(name='req0', requirement='requirement', design=['Diffusion.md'],
                                    issues=issues, issues_line=1)
        req0.details = [moosesqa.Detail(name='req0-0', detail='detail')] # add detail so Req. is testable

        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])

        self.assertEqual(len(cm.output), 4) # only 4 errors
        self.assertIn("(11600c68be9e8f77e4870d7b8efb26e5ada10a5a23423)", cm.output[0])
        self.assertIn(":1", cm.output[0])

        self.assertIn("(11600)", cm.output[1])
        self.assertIn(":1", cm.output[1])

        self.assertIn("(26exada10a5a)", cm.output[2])
        self.assertIn(":1", cm.output[2])

        self.assertIn("(#1234c)", cm.output[3])
        self.assertIn(":1", cm.output[3])

    def testFiles(self):
        req0 = moosesqa.Requirement(name='req0', issues=['#1234'], requirement='requirement',
                                    design=['_not_a_file_name_.md'])
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Unable to locate 'design' document", cm.output[0])

        req0.design = ['Diffusion.md']
        req0.verification = ['_not_a_file_name_.md']
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Unable to locate 'verification' document", cm.output[0])

        req0.verification = None
        req0.validation = ['_not_a_file_name_.md']
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Unable to locate 'validation' document", cm.output[0])

    def testTestable(self):
        req0 = moosesqa.Requirement(name='req0', requirement='requirement', design=['Diffusion.md'],
                                    issues=['#1234'], skip=True)
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0])
        self.assertIn("Test will not execute because it is marked as skipped or deleted", cm.output[0])

    def testCollections(self):
        req0 = moosesqa.Requirement(name='req0',
                                    requirement='requirement', design=['Diffusion.md'], issues=['#1234'],
                                    specification=moosesqa.TestSpecification(), collections={'test'})
        req1 = moosesqa.Requirement(name='req1',
                                    requirement='requirement2', design=['Diffusion.md'], issues=['#1234'],
                                    specification=moosesqa.TestSpecification(), collections={'test2'})
        with self.assertLogs(level='ERROR') as cm:
            moosesqa.check_requirements([req0, req1], allowed_collections={'test'})
        self.assertIn("Invalid collection names found: test2", cm.output[0])

if __name__ == '__main__':
    unittest.main(verbosity=2)
