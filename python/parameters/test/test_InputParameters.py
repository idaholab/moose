#!/usr/bin/env python
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import sys
import re
import unittest
from parameters import InputParameters

class TestInputParameters(unittest.TestCase):

    def testAdd(self):
        params = InputParameters()
        params.add('foo')
        self.assertEqual(list(params.keys()), ['foo'])
        self.assertFalse(params.isValid('foo'))
        self.assertIn('foo', params)
        self.assertIsNone(params.get('foo'))
        self.assertTrue(params.hasParameter('foo'))

        with self.assertLogs(level='WARNING') as log:
            params.add('foo')
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot add parameter, the parameter 'foo' already exists.", log.output[0])

    def testContains(self):
        params = InputParameters()
        params.add('foo')
        self.assertIn('foo', params)
        self.assertNotIn('bar', params)

    def testIAdd(self):
        params = InputParameters()
        params.add('foo')

        params2 = InputParameters()
        params2.add('bar')

        params += params2
        self.assertIn('foo', params)
        self.assertIn('bar', params)

    def testItems(self):
        params = InputParameters()
        params.add('foo', 1949)
        params.add('bar', 1980)

        gold = [('foo', 1949), ('bar', 1980)]
        for i, (k, v) in enumerate(params.items()):
            self.assertEqual(k, gold[i][0])
            self.assertEqual(v, gold[i][1])

    def testValues(self):
        params = InputParameters()
        params.add('foo', 1949)
        params.add('bar', 1980)

        gold = [1949, 1980]
        for i, v in enumerate(params.values()):
            self.assertEqual(v, gold[i])

    def testKeys(self):
        params = InputParameters()
        params.add('foo', 1949)
        params.add('bar', 1980)

        gold = ['foo', 'bar']
        for i, v in enumerate(params.keys()):
            self.assertEqual(v, gold[i])

    def testRemove(self):
        params = InputParameters()
        params.add('foo')
        self.assertTrue(params.hasParameter('foo'))
        params.remove('foo')
        self.assertFalse(params.hasParameter('foo'))

        with self.assertLogs(level='WARNING') as log:
            params.remove('bar')
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot remove parameter, the parameter 'bar' does not exist", log.output[0])

    def testIsValid(self):
        params = InputParameters()
        params.add('foo')
        self.assertFalse(params.isValid('foo'))
        params.set('foo', 1980)
        self.assertTrue(params.isValid('foo'))

        with self.assertLogs(level='WARNING') as log:
            self.assertIsNone(params.isValid('bar'))
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot determine if the parameters is valid, the parameter 'bar' does not exist", log.output[0])

    def testSetDefault(self):
        params = InputParameters()
        params.add('foo')
        self.assertIsNone(params.get('foo'))
        params.setDefault('foo', 1980)
        self.assertEqual(params.get('foo'), 1980)

        params.add('bar', default=1980)
        params.setDefault('bar', 1949)
        self.assertEqual(params.get('bar'), 1980)
        self.assertEqual(params.getDefault('bar'), 1949)

        with self.assertLogs(level='WARNING') as log:
            params.setDefault('other', 1980)
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot set default, the parameter 'other' does not exist", log.output[0])

    def testGetDefault(self):
        params = InputParameters()
        params.add('foo', default=42)
        params.set('foo', 54)
        self.assertEqual(params.getDefault('foo'), 42)

        with self.assertLogs(level='WARNING') as log:
            self.assertIsNone(params.getDefault('bar'))
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot get default, the parameter 'bar' does not exist", log.output[0])

    def testIsDefault(self):
        params = InputParameters()
        params.add('foo', default=1949)
        self.assertTrue(params.isDefault('foo'))
        params.set('foo', 1980)
        self.assertFalse(params.isDefault('foo'))

        with self.assertLogs(level='WARNING') as log:
            self.assertIsNone(params.isDefault('bar'))
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot determine if the parameter is default, the parameter 'bar' does not exist", log.output[0])

    def testSet(self):
        params = InputParameters()
        params.add('foo')
        params.set('foo', 42)
        self.assertEqual(list(params.keys()), ['foo'])
        self.assertTrue(params.isValid('foo'))
        self.assertIn('foo', params)
        self.assertIsNotNone(params.get('foo'))
        self.assertEqual(params.get('foo'), 42)
        self.assertTrue(params.hasParameter('foo'))

        with self.assertLogs(level='WARNING') as log:
            params.set('bar', 1980)
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot set value, the parameter 'bar' does not exist", log.output[0])

        # Sub-options
        params2 = InputParameters()
        params2.add('bar')
        params.add('sub', params2)
        params.set('sub', {'bar':2013})
        self.assertEqual(params2.get('bar'), 2013)
        self.assertEqual(params.get('sub').get('bar'), 2013)

        params2.set('bar', 1954)
        self.assertEqual(params2.get('bar'), 1954)
        self.assertEqual(params.get('sub').get('bar'), 1954)

        params3 = InputParameters()
        params3.add('bar', default=2011)
        params.set('sub', params3)
        self.assertEqual(params2.get('bar'), 1954)
        self.assertEqual(params3.get('bar'), 2011)
        self.assertEqual(params.get('sub').get('bar'), 2011)

        params.set('sub', bar=1944)
        self.assertEqual(params2.get('bar'), 1954)
        self.assertEqual(params3.get('bar'), 1944)
        self.assertEqual(params.get('sub').get('bar'), 1944)

        # More errors
        with self.assertLogs(level='WARNING') as log:
            params.set('foo', 1980, this='that')
        self.assertEqual(len(log.output), 1)
        self.assertIn("Key, value pairs are not allowed when setting the 'foo' parameter.", log.output[0])

        with self.assertLogs(level='WARNING') as log:
            params.set('foo', 1980, 2011)
            params.set('foo')

        self.assertEqual(len(log.output), 2)
        self.assertIn("A single second argument is required for the 'foo' parameter.", log.output[0])
        self.assertIn("A single second argument is required for the 'foo' parameter.", log.output[1])

        with self.assertLogs(level='WARNING') as log:
            params.set('sub', 1980, bar=1948)
        self.assertEqual(len(log.output), 1)
        self.assertIn("Key, value pairs are not allowed when setting the 'sub' parameter with a supplied dict argument.", log.output[0])

        with self.assertLogs(level='WARNING') as log:
            params.set('sub', 1980)
        self.assertEqual(len(log.output), 1)
        self.assertIn("The second argument for the 'sub' parameter must be a dict() or InputParametrs object", log.output[0])

    def testGet(self):
        params = InputParameters()
        params.add('foo', default=1980)
        self.assertEqual(params.get('foo'), 1980)

        with self.assertLogs(level='WARNING') as log:
            self.assertIsNone(params.get('bar'))
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot get value, the parameter 'bar' does not exist", log.output[0])

    def testHasParameter(self):
        params = InputParameters()
        params.add('foo')
        self.assertTrue(params.hasParameter('foo'))
        self.assertFalse(params.hasParameter('bar'))

    def testUpdate(self):
        params = InputParameters()
        params.add('foo')
        params.update(foo=1980)
        self.assertEqual(params.get('foo'), 1980)

        params2 = InputParameters()
        params2.add('foo', 2013)

        params.update(params2)
        self.assertEqual(params.get('foo'), 2013)

        with self.assertLogs(level='WARNING') as log:
            params.update(foo=2011, bar=2013)
        self.assertEqual(len(log.output), 1)
        self.assertIn("The following parameters do not exist: bar", log.output[0])

    def testErrorMode(self):
        params = InputParameters(InputParameters.ErrorMode.WARNING)
        with self.assertLogs(level='WARNING') as log:
            self.assertIsNone(params.isValid('bar'))
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot determine if the parameters is valid, the parameter 'bar' does not exist", log.output[0])

        params = InputParameters(InputParameters.ErrorMode.ERROR)
        with self.assertLogs(level='ERROR') as log:
            self.assertIsNone(params.isValid('bar'))
        self.assertEqual(len(log.output), 1)
        self.assertIn("Cannot determine if the parameters is valid, the parameter 'bar' does not exist", log.output[0])

        params = InputParameters(InputParameters.ErrorMode.EXCEPTION)
        with self.assertRaises(InputParameters.InputParameterException):
            with self.assertLogs(level='CRITICAL') as log:
                self.assertIsNone(params.isValid('bar'))
            self.assertEqual(len(log.output), 1)
            self.assertIn("Cannot determine if the parameters is valid, the parameter 'bar' does not exist", log.output[0])

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
