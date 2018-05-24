#!/usr/bin/env python2

#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase
from TestHarness.CSVDiffer import CSVDiffer
import unittest

class TestHarnessTester(TestHarnessTestCase):
    """
    Unit tests for the CSVDiffer class
    """
    def testCSVDiffer(self):
        # Test for success and ignoring newlines
        d = CSVDiffer(None, [])
        d.addCSVPair('out.csv', 'col1,col2\n1,2\n1,2', 'col1,col2\n1,2\n1,2')
        d.addCSVPair('out2.csv', 'col1,col2\n \n1,2\n1,2', 'col1,col2\n1,2\n\t\n1,2')
        d.addCSVPair('out3.csv', 'col1,col2\n1,2\n1,2\n\n', 'col1,col2\n1,2\n1,2')
        msg = d.diff()
        self.assertEqual(d.getNumErrors(), 0)
        self.assertEquals(msg, "")

        # Test for different number of columns
        d = CSVDiffer(None, [])
        d.addCSVPair('out.csv', 'col1,col2\n1,2\n1,2', 'col1,col2,col3\n1,2,3\n1,2,3')
        msg = d.diff()
        self.assertEqual(d.getNumErrors(), 1)
        self.assertIn("Header 'col3' is missing", msg)

        # Test for different column lengths
        d = CSVDiffer(None, [])
        d.addCSVPair('out1.csv', 'col1,col2\n1,2\n1,2', 'col1,col2,col3\n1,2,3\n1,2,3')
        d.addCSVPair('out2.csv', 'col1,col2\n1,2\n1,2\n3,4', 'col1,col2\n1,2\n1,2')
        msg = d.diff()
        self.assertEqual(d.getNumErrors(), 2)
        self.assertIn("Header 'col3' is missing", msg)
        self.assertIn("Columns with header 'col2' aren't the same length", msg)

        # Test for absolute zero logic
        d = CSVDiffer(None, [])
        d.addCSVPair('out1.csv', 'col1,col2\n1,2\n1,2', 'col1,col2\n1,2\n1,2.1')
        d.addCSVPair('out2.csv', 'col1,col2\n1,2\n1,2\n0,-1e-13', 'col1,col2\n1,2\n1,2\n1e-12,1e-13')
        d.addCSVPair('out3.csv', 'col1,col2\n1,2\n1,2\n0,0', 'col1,col2\n1,2\n1,2\n1e-4,1e-13')
        msg = d.diff()
        self.assertEqual(d.getNumErrors(), 2)
        self.assertIn("out1.csv: The values in column \"col2\" don't match", msg)
        self.assertIn("out3.csv: The values in column \"col1\" don't match", msg)

        # Test relative tolerance
        d = CSVDiffer(None, [])
        d.addCSVPair('out1.csv', 'col1,col2\n1,2\n1,2', 'col1,col2\n1,2\n1,2.1')
        d.addCSVPair('out2.csv', 'col1,col2\n1,-2\n1,-2', 'col1,col2\n1,-2\n1,-2.00000000001')
        d.addCSVPair('out3.csv', 'col1,col2\n1,2\n1,2', 'col1,col2\n1,2\n1.00001,2.0001')
        msg = d.diff()
        self.assertEqual(d.getNumErrors(), 3)
        self.assertIn("out1.csv: The values in column \"col2\" don't match", msg)
        self.assertIn("out3.csv: The values in column \"col2\" don't match", msg)
        self.assertIn("out3.csv: The values in column \"col1\" don't match", msg)

        # test file does not exist
        d = CSVDiffer('qwertyuiop', ['out.csv'])
        self.assertEqual(d.getNumErrors(), 1)
        self.assertIn("File does not exist", d.msg)

        # tests for override parameters
        d = CSVDiffer(None, [], 1e-11, 5.5e-6, 'gold', ['col1','col2','col3'], ['1e-2','1e-2','1e-2'], ['1e-10','1e-10','1e-10'])
        d.addCSVPair('out1.csv', 'col1,col2\n1,2\n1,2',' col1,col2\n1,2\n1,2')
        msg = d.diff()
        self.assertEqual(d.getNumErrors(), 1)
        self.assertIn("In all CSV files: Variable 'col3' in custom_columns is missing", msg)

if __name__ == '__main__':
    unittest.main(verbosity=2)
