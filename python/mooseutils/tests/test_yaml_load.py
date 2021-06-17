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
import tempfile
from mooseutils.yaml_load import yaml_load, yaml_write, IncludeYamlFile

class TestYamlLoad(unittest.TestCase):
    """
    Test that the size function returns something.
    """
    def testLoad(self):
        data = yaml_load('bar.yml')
        self.assertEqual(data[0], 3.6)
        self.assertEqual(data[1], [1,2,3])
        self.assertEqual(data[2], 'item')
        self.assertEqual(data[3], 'other')

    def testInclude(self):
        data = yaml_load('foo.yml')
        self.assertEqual(data['a'], 1)
        self.assertEqual(data['b'], [1.43, 543.55])
        self.assertEqual(data['c'][0], 3.6)
        self.assertEqual(data['c'][1], [1,2,3])
        self.assertEqual(data['c'][2], 'item')
        self.assertEqual(data['c'][3], 'other')

    def testError(self):
        with self.assertRaises(IOError) as e:
            yaml_load('unknown.yml')
        self.assertIn("No such file or directory: 'unknown.yml'", str(e.exception))

        with self.assertRaises(IOError) as e:
            yaml_load('foo_error.yml')
        self.assertIn("Unknown include file 'unknown.yml' on line 5 of foo_error.yml",
                      str(e.exception))

    def testIncludeWithKey(self):
        data = yaml_load('foo.yml')
        self.assertEqual(data['d'], 1980)
        self.assertEqual(data['e'], ['Edward', 'Bonnie'])
        self.assertEqual(data['f'], [1906])

class TestYamlWrite(unittest.TestCase):
    def setUp(self):
        _, self._tmp = tempfile.mkstemp(suffix='.yml', dir=os.path.dirname(__file__), text=True)

    def tearDown(self):
        os.remove(self._tmp)

    def testWrite(self):

        # Load file without processing includes
        data = yaml_load('foo.yml', include=False)
        self.assertIsInstance(data['c'], IncludeYamlFile)
        self.assertIsInstance(data['d'], IncludeYamlFile)
        self.assertIsInstance(data['e'], IncludeYamlFile)
        self.assertIsInstance(data['f'], IncludeYamlFile)

        # Write data out
        yaml_write(self._tmp, data)
        self.assertTrue(os.path.isfile(self._tmp))

        # Load the newly written file and process includes
        # (the results should be as in TestYamlLoad.testInclude)
        data = yaml_load(self._tmp)
        self.assertEqual(data['a'], 1)
        self.assertEqual(data['b'], [1.43, 543.55])
        self.assertEqual(data['c'][0], 3.6)
        self.assertEqual(data['c'][1], [1,2,3])
        self.assertEqual(data['c'][2], 'item')
        self.assertEqual(data['c'][3], 'other')

        # Test that output is exact as input
        with open('foo.yml', 'r') as fid:
            in_data = fid.read()
        with open(self._tmp, 'r') as fid:
            out_data = fid.read()
        self.assertEqual(in_data, out_data)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
