#!/usr/bin/env python3
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
import logging
from parameters import Parameter

class TestParameter(unittest.TestCase):

    def testMinimal(self):
        opt = Parameter('foo')
        self.assertEqual(opt.name, 'foo')
        self.assertIsNone(opt.default)
        self.assertIsNone(opt.value)

    def testValue(self):
        opt = Parameter('foo')
        self.assertEqual(opt.name, 'foo')
        self.assertIsNone(opt.default)
        self.assertIsNone(opt.value)

        self.value = 12345
        self.assertEqual(self.value, 12345)

    def testDefault(self):
        opt = Parameter('foo', default=12345)
        self.assertEqual(opt.name, 'foo')
        self.assertEqual(opt.default, 12345)
        self.assertEqual(opt.value, 12345)

        opt.value = '12345'
        self.assertEqual(opt.default, 12345)
        self.assertEqual(opt.value, '12345')

        opt = Parameter('bar', default=1980, vtype=int)
        with self.assertLogs(level=logging.WARNING) as cm:
            opt.default = 'nope'
        self.assertEqual(len(cm.output), 1)
        self.assertIn("'bar' must be of type (<class 'int'>,) but <class 'str'> provided.", cm.output[0])

    def testAllow(self):
        opt = Parameter('foo', allow=(1, 'two'))
        self.assertIsNone(opt.default)
        self.assertIsNone(opt.value)

        opt.value = 1
        self.assertEqual(opt.value, 1)

        opt.value = 'two'
        self.assertEqual(opt.value, 'two')

        with self.assertLogs(level=logging.WARNING) as cm:
            opt.value = 4
        self.assertEqual(len(cm.output), 1)
        self.assertIn("Attempting to set 'foo' to a value of 4 but only the following are allowed: (1, 'two')", cm.output[0])

    def testType(self):
        opt = Parameter('foo', vtype=int)
        self.assertIsNone(opt.default)
        self.assertIsNone(opt.value)

        opt.value = 1
        self.assertEqual(opt.value, 1)

        with self.assertLogs(level=logging.WARNING) as cm:
            opt.value = 's'
        self.assertEqual(len(cm.output), 1)
        self.assertIn("'foo' must be of type (<class 'int'>,) but <class 'str'> provided.", cm.output[0])

    def testTypeWithAllow(self):

        opt = Parameter('foo', vtype=int, allow=(1,2))
        self.assertIsNone(opt.default)
        self.assertIsNone(opt.value)

        opt.value = 2
        self.assertEqual(opt.value, 2)

        opt.value = 1
        self.assertEqual(opt.value, 1)

        with self.assertLogs(level=logging.WARNING) as cm:
            opt.value = 4
        self.assertEqual(len(cm.output), 1)

        self.assertIn("Attempting to set 'foo' to a value of 4 but only the following are allowed: (1, 2)", cm.output[0])
        self.assertEqual(opt.value, 1)

    def testAllowWithTypeError(self):

        with self.assertRaises(TypeError) as e:
            Parameter('foo', allow='wrong')
        self.assertIn("The supplied 'allow' argument must be a 'tuple', but <class 'str'> was provided.", str(e.exception))

        with self.assertRaises(TypeError) as e:
            Parameter('foo', vtype=int, allow=(1,'2'))
        self.assertIn("The supplied 'allow' argument must be a 'tuple' of (<class 'int'>,) items, but a <class 'str'> item was provided.", str(e.exception))

    def testArray(self):
        opt = Parameter('foo', default=(1,2), array=True)
        self.assertEqual(opt._Parameter__array, True)
        self.assertEqual(opt.value, (1,2))

        with self.assertLogs(level=logging.WARNING) as cm:
            opt.value = 4
        self.assertIn("'foo' was defined as an array, which require <class 'tuple'> for assignment, but a <class 'int'> was provided.", cm.output[0])

        opt.value = (3, 4)
        self.assertEqual(opt.value, (3,4))

        opt.value = ('1', )
        self.assertEqual(opt.value, ('1',))

        opt = Parameter('foo', vtype=int, array=True)
        self.assertEqual(opt._Parameter__array, True)
        self.assertIsNone(opt.value)

        with self.assertLogs(level=logging.WARNING) as cm:
            opt.value = 4
        self.assertEqual(len(cm.output), 1)
        self.assertIn("'foo' was defined as an array, which require <class 'tuple'> for assignment, but a <class 'int'> was provided.", cm.output[0])

        with self.assertLogs(level=logging.WARNING) as cm:
            opt.value = ('x', )
        self.assertEqual(len(cm.output), 1)
        self.assertIn("The values within 'foo' must be of type (<class 'int'>,) but <class 'str'> provided.", cm.output[0])
        self.assertIsNone(opt.value)

        opt.value = (1, )
        self.assertEqual(opt.value, (1,))

    def testSize(self):
        opt = Parameter('foo', size=4)
        self.assertEqual(opt._Parameter__array, True)
        self.assertEqual(opt._Parameter__size, 4)

        with self.assertLogs(level=logging.WARNING) as cm:
            opt.value = (1,2,3)
        self.assertIn("'foo' was defined as an array with length 4 but a value with length 3 was provided.", cm.output[0])

    def testDoc(self):
        opt = Parameter('foo', doc='This is foo, not bar.')
        self.assertEqual(opt.doc, 'This is foo, not bar.')

        opt = Parameter('foo', doc=u'This is foo, not bar.')
        self.assertEqual(opt.doc, u'This is foo, not bar.')

        with self.assertRaises(TypeError) as e:
            Parameter('foo', doc=42)
        self.assertIn("The supplied 'doc' argument must be a 'str', but <class 'int'> was provided.", str(e.exception))

    def testName(self):
        opt = Parameter('foo')
        self.assertEqual(opt.name, 'foo')

        opt = Parameter(u'foo')
        self.assertEqual(opt.name, u'foo')

        with self.assertRaises(TypeError) as e:
            Parameter(42)
        self.assertIn("The supplied 'name' argument must be a 'str', but <class 'int'> was provided.", str(e.exception))

    def testRequired(self):
        opt = Parameter('year', required=True)
        self.assertEqual(opt.required, True)

        with self.assertLogs(level=logging.WARNING) as cm:
            retcode = opt.validate()
        self.assertEqual(retcode, 1)
        self.assertEqual(len(cm.output), 1)
        self.assertIn("The Parameter 'year' is marked as required, but no value is assigned.", cm.output[0])

        with self.assertRaises(TypeError) as e:
            Parameter('year', required="wrong")
        self.assertIn("The supplied 'required' argument must be a 'bool', but <class 'str'> was provided.", str(e.exception))

        opt.value = 1980
        self.assertEqual(opt.validate(), 0)

    def testSetDefault(self):
        opt = Parameter('year', default=1980)
        self.assertEqual(opt.value, 1980)
        self.assertEqual(opt.default, 1980)

        opt.default = 1949
        self.assertEqual(opt.value, 1980)
        self.assertEqual(opt.default, 1949)

        opt = Parameter('year')
        self.assertEqual(opt.value, None)
        opt.default = 1949
        self.assertEqual(opt.value, 1949)
        self.assertEqual(opt.default, 1949)

    def testPrivate(self):
        opt = Parameter('year')
        self.assertEqual(opt.private, False)

        opt = Parameter('year', private=True)
        self.assertEqual(opt.private, True)

        opt = Parameter('_year', private=False)
        self.assertEqual(opt.private, False)

        opt = Parameter('_year')
        self.assertEqual(opt.private, True)

    def testToString(self):
        opt = Parameter('year')
        s = str(opt)
        self.assertIn('Value:   None', s)
        self.assertNotIn('Default', s)
        self.assertNotIn('Type', s)
        self.assertNotIn('Allow', s)

        opt = Parameter('year', default=1980, vtype=int, allow=(1949, 1954, 1977, 1980))
        opt.value = 1954
        s = str(opt)
        self.assertIn('Value:   1954', s)
        self.assertIn('Default: 1980', s)
        self.assertIn("Type(s): ('int',)", s)
        self.assertIn('Allow:   (1949, 1954, 1977, 1980)', s)

        opt = Parameter('year', default=1980, doc="The best year.")
        s = str(opt)
        self.assertIn("best", s)

    def testVerify(self):
        opt = Parameter('year', verify=(lambda v: v > 1980, "The year must be greater than 1980."))
        self.assertEqual(opt.value, None)
        opt.value = 1990
        self.assertEqual(opt.value, 1990)

        with self.assertLogs(level=logging.WARNING) as cm:
            opt.value = 1949
        self.assertEqual(len(cm.output), 1)
        self.assertIn("Verify function failed with the given value of 1949\nThe year must be greater than 1980.", cm.output[0])

        with self.assertRaises(TypeError) as e:
            Parameter('year', verify="wrong")
        self.assertIn("The supplied 'verify' argument must be a 'tuple' with callable function and 'str' error message, but <class 'str'> was provided.", str(e.exception))

        with self.assertRaises(TypeError) as e:
            Parameter('year', verify=("wrong", 1, 2))
        self.assertIn("The supplied 'verify' argument must be a 'tuple' with two items a callable function and 'str' error message, but <class 'tuple'> with 3 items was provided.", str(e.exception))

        with self.assertRaises(TypeError) as e:
            Parameter('year', verify=("wrong", "message"))
        self.assertIn("The first item in the 'verify' argument tuple must be a callable function with a single argument, but <class 'str'> was provided", str(e.exception))

        with self.assertRaises(TypeError) as e:
            Parameter('year', verify=(lambda x,y: True, "message"))
        self.assertIn("The first item in the 'verify' argument tuple must be a callable function with a single argument, but <class 'function'> was provided that has 2 arguments.", str(e.exception))

        with self.assertRaises(TypeError) as e:
            Parameter('year', verify=(lambda v: True, 42))
        self.assertIn("The second item in the 'verify' argument tuple must be a string, but <class 'int'> was provided", str(e.exception))

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2, buffer=True)
