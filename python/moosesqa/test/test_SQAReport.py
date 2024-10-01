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
from moosesqa import SQAReport, SilentRecordHandler, LogHelper

class TestSQAReport(unittest.TestCase):
    def testStatus(self):
        self.assertEqual(SQAReport.Status.PASS, 0)
        self.assertEqual(SQAReport.Status.WARNING, 1)
        self.assertEqual(SQAReport.Status.ERROR, 2)

    def testReport(self):
        with self.assertRaises(NotImplementedError):
            r = SQAReport()
            r.execute()

        logger = logging.getLogger('moosesqa')
        class TestReport(SQAReport):
            def execute(self):
                logger = LogHelper('moosesqa', 'log_error', log_critical=logging.CRITICAL,
                                   log_warning=logging.WARNING)
                logger.log('log_warning', 'warning message')
                logger.log('log_error', 'error message')
                logger.log('log_critical', 'critical message')
                return logger

        report = TestReport()
        r = str(report.getReport())
        self.assertEqual(report.status, SQAReport.Status.ERROR)
        self.assertIn('log_warning: 1', r)
        self.assertIn('log_error: 1', r)
        self.assertIn('log_critical: 1', r)
        self.assertNotIn('warning message', r)
        self.assertIn('error message', r)
        self.assertIn('critical message', r)

        report = TestReport(show_warning=True)
        r = str(report.getReport())
        self.assertEqual(report.status, SQAReport.Status.ERROR)
        self.assertIn('log_warning: 1', r)
        self.assertIn('log_error: 1', r)
        self.assertIn('log_critical: 1', r)
        self.assertIn('warning message', r)
        self.assertIn('error message', r)
        self.assertIn('critical message', r)

        report = TestReport(show_error=False)
        r = str(report.getReport())
        self.assertEqual(report.status, SQAReport.Status.ERROR)
        self.assertIn('log_warning: 1', r)
        self.assertIn('log_error: 1', r)
        self.assertIn('log_critical: 1', r)
        self.assertNotIn('warning message', r)
        self.assertNotIn('error message', r)
        self.assertIn('critical message', r)

        report = TestReport(show_critical=False)
        r = str(report.getReport())
        self.assertEqual(report.status, SQAReport.Status.ERROR)
        self.assertIn('log_warning: 1', r)
        self.assertIn('log_error: 1', r)
        self.assertIn('log_critical: 1', r)
        self.assertNotIn('warning message', r)
        self.assertIn('error message', r)
        self.assertNotIn('critical message', r)

    @mock.patch('mooseutils.colorText', side_effect=lambda t, c, **kwargs: '{}::{}'.format(c, t))
    def testColorText(self, color_text):
        r = SQAReport()
        txt = r._colorTextByStatus(1, SQAReport.Status.PASS)
        self.assertEqual(txt, 'LIGHT_GREEN::1')
        txt = r._colorTextByStatus(1, SQAReport.Status.ERROR)
        self.assertEqual(txt, 'LIGHT_RED::1')
        txt = r._colorTextByStatus(1, SQAReport.Status.WARNING)
        self.assertEqual(txt, 'LIGHT_YELLOW::1')

        txt = r._colorTextByMode(1, logging.ERROR)
        self.assertEqual(txt, 'LIGHT_RED::1')
        txt = r._colorTextByMode(1, logging.WARNING)
        self.assertEqual(txt, 'LIGHT_YELLOW::1')


    @mock.patch('mooseutils.colorText', side_effect=lambda t, c, **kwargs: '{}::{}'.format(c, t))
    def testGetStatusText(self, color_text):
        r = SQAReport()
        txt = r._getStatusText(SQAReport.Status.PASS)
        self.assertEqual(txt, 'LIGHT_GREEN::OK')
        txt = r._getStatusText(SQAReport.Status.WARNING)
        self.assertEqual(txt, 'LIGHT_YELLOW::WARNING')
        txt = r._getStatusText(SQAReport.Status.ERROR)
        self.assertEqual(txt, 'LIGHT_RED::FAIL')

if __name__ == '__main__':
    unittest.main(verbosity=2)
