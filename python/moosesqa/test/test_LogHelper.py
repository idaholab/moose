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
import logging
import moosesqa

class TestLogHelper(unittest.TestCase):
    def testLogs(self):
        log = moosesqa.LogHelper('helper', 'log_hidden')

        with self.assertLogs(level='ERROR') as cm:
            log.log('log_hidden', 'message')
        self.assertIn('ERROR:helper:message', cm.output[0])

        log.setLevel('log_hidden', logging.WARNING)
        with self.assertLogs(level='WARNING') as cm:
            log.log('log_hidden', 'message')
        self.assertIn('WARNING:helper:message', cm.output[0])

        log.setLevel('log_hidden', None)
        with self.assertRaises(AssertionError):
            with self.assertLogs() as cm:
                log.log('log_hidden', 'message')

        log = moosesqa.LogHelper('helper', 'log_hidden', default=logging.DEBUG)
        with self.assertLogs(level='DEBUG') as cm:
            log.log('log_hidden', 'message')
        self.assertIn('DEBUG:helper:message', cm.output[0])

    def testFormat(self):
        log = moosesqa.LogHelper('helper', 'log_hidden')
        with self.assertLogs(level='ERROR') as cm:
            log.log('log_hidden', '{} {foo}', 'foo', foo='bar')
        self.assertIn('ERROR:helper:foo bar', cm.output[0])

    def testKwargs(self):
        log = moosesqa.LogHelper('helper', log_hidden=logging.WARNING)
        with self.assertLogs(level='WARNING') as cm:
            log.log('log_hidden', 'message')
        self.assertIn('WARNING:helper:message', cm.output[0])

    def testCounts(self):
        log = moosesqa.LogHelper('helper', log_hidden=logging.WARNING)
        log.log('log_hidden', 'message')
        log.log('log_hidden', 'message')
        self.assertEqual(log.counts['log_hidden'], 2)



if __name__ == '__main__':
    unittest.main(verbosity=2)
