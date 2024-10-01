#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import tempfile
import os

from TestHarness import OutputInterface

class TestHarnessTester(unittest.TestCase):
    def testInMemory(self):
        # Empty state
        oi = OutputInterface()
        self.assertIsNone(oi.getSeparateOutputFilePath())
        self.assertFalse(oi.hasOutput())
        self.assertEqual(oi.output, '')
        self.assertEqual(oi.getOutput(), '')

        # Add output
        output = 'foo'
        oi.setOutput(output)
        self.assertIsNone(oi.getSeparateOutputFilePath())
        self.assertTrue(oi.hasOutput())
        self.assertEqual(oi.getOutput(), output)

        # Clear output
        oi.clearOutput()
        self.assertFalse(oi.hasOutput())

        # Append output empty
        output = 'bar'
        oi.appendOutput(output)
        self.assertTrue(oi.hasOutput())
        self.assertEqual(oi.getOutput(), output)

        # Append more
        oi.appendOutput(output)
        self.assertTrue(oi.hasOutput())
        self.assertEqual(oi.getOutput(), output + output)

        # Reset
        output = 'foo'
        oi.setOutput(output)
        self.assertTrue(oi.hasOutput())
        self.assertEqual(oi.getOutput(), output)
        # And then append
        for i in range(2):
            oi.appendOutput(output)
        self.assertTrue(oi.hasOutput())
        self.assertEqual(oi.getOutput(), output * 3)

    def testSeparate(self):
        with tempfile.TemporaryDirectory() as dir:
            output_file = os.path.join(dir, 'output')

            # Empty state
            oi = OutputInterface()
            oi.setSeparateOutputPath(output_file)
            self.assertEqual(oi.getSeparateOutputFilePath(), output_file)
            self.assertFalse(os.path.exists(output_file))
            self.assertFalse(oi.hasOutput())
            self.assertEqual(oi.output, '')
            self.assertEqual(oi.getOutput(), '')

            # Add output
            output = 'foo'
            oi.setOutput(output)
            self.assertTrue(os.path.exists(output_file))
            self.assertTrue(oi.hasOutput())
            self.assertEqual(oi.getOutput(), output)

            # Clear output
            oi.clearOutput()
            self.assertFalse(os.path.exists(output_file))
            self.assertFalse(oi.hasOutput())

            # Append output empty
            output = 'bar'
            oi.appendOutput(output)
            self.assertTrue(os.path.exists(output_file))
            self.assertTrue(oi.hasOutput())
            self.assertEqual(oi.getOutput(), output)

            # Append more
            oi.appendOutput(output)
            self.assertTrue(oi.hasOutput())
            self.assertTrue(os.path.exists(output_file))
            self.assertEqual(oi.getOutput(), output + output)

            # Reset
            output = 'foo'
            oi.setOutput(output)
            self.assertTrue(os.path.exists(output_file))
            self.assertTrue(oi.hasOutput())
            self.assertEqual(oi.getOutput(), output)
            # And then append
            for i in range(2):
                oi.appendOutput(output)
            self.assertTrue(oi.hasOutput())
            self.assertEqual(oi.getOutput(), output * 3)

    def testBadOutput(self):
        oi = OutputInterface()

        null_chars = 'foobar\nwith a dirty \0and another dirty\x00'
        null_replaced = null_chars.replace('\0', 'NULL').replace('\x00', 'NULL')

        # Set null characters
        oi.setOutput(null_chars)
        failures = oi.sanitizeOutput()
        self.assertEqual(failures, ['NULL output'])
        self.assertEqual(oi.getOutput(), null_replaced)

        # Set null characters without sanitize
        oi.setOutput(null_chars)
        with self.assertRaises(OutputInterface.BadOutputException) as e:
            oi.getOutput()
        self.assertEqual(e.exception.errors, ['NULL output'])
        self.assertEqual(str(e.exception), 'Bad output detected: NULL output')
