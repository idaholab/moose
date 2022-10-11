#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import unittest
import mock
import logging

import MooseDocs
import mooseutils
from moosesqa import SQAReport, SQADocumentReport, LogHelper

@unittest.skipIf(mooseutils.git_version() < (2,11,4), "Git version must at least 2.11.4")
class TestSQADocumentReport(unittest.TestCase):
    def setUp(self):
        SQADocumentReport.FILE_CACHE = MooseDocs.PROJECT_FILES

    @mock.patch('mooseutils.colorText', side_effect=lambda t, c, **kwargs: t)
    def testReport(self, color_text):

        # PASS
        reporter = SQADocumentReport(required_documents=['rtm', 'google'], rtm='moose_rtm.md', google='https://www.google.com')
        r = reporter.getReport()
        self.assertEqual(reporter.status, SQAReport.Status.PASS)
        self.assertIn('log_rtm: 0', r)
        self.assertIn('log_google: 0', r)

        # ERROR with missing doc
        reporter = SQADocumentReport(required_documents=['rtm', 'google'], rtm='moose_rtm.md')
        r = reporter.getReport()
        self.assertEqual(reporter.status, SQAReport.Status.ERROR)
        self.assertIn('log_rtm: 0', r)
        self.assertIn('log_google: 1', r)

        # WARNING with missing doc
        reporter = SQADocumentReport(required_documents=['rtm', 'google'], rtm='moose_rtm.md', log_google='WARNING')
        r = reporter.getReport()
        self.assertEqual(reporter.status, SQAReport.Status.WARNING)
        self.assertIn('log_rtm: 0', r)
        self.assertIn('log_google: 1', r)

if __name__ == '__main__':
    unittest.main(verbosity=2)
