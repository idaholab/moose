#!/usr/bin/env python2
import unittest
from mooseutils.yaml_load import yaml_load

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
            yaml_load('unkown.yml')
        self.assertIn("No such file or directory: 'unkown.yml'", str(e.exception))

        with self.assertRaises(IOError) as e:
            yaml_load('foo_error.yml')
        self.assertIn("Unknown include file 'unknown.yml' on line 5 of foo_error.yml",
                      str(e.exception))


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True, exit=False)
