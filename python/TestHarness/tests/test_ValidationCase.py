#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: disable
import unittest
from TestHarness import ValidationCase
from TestHarness.validation import NoTestsDefined, TestMissingResults, TestRunException
from FactorySystem.InputParameters import InputParameters

class TestValidationCase(unittest.TestCase):
    def testNoTests(self):
        test = ValidationCase()
        with self.assertRaises(NoTestsDefined):
            test.run()

    def testAddResult(self):
        test_message = 'What a cool message'

        for validation in [True, False]:
            for status in ValidationCase.Status.list():
                class Test(ValidationCase):
                    def test(self):
                        self.addResult(status, test_message, validation=validation)

                test = Test()
                test.run()

                results = test.results
                self.assertEqual(len(results), 1)

                result = results[0]
                self.assertEqual(result.message, test_message)
                self.assertEqual(result.status, status)
                self.assertEqual(result.test, 'Test.test')
                self.assertEqual(result.validation, validation)

    def testMissingResult(self):
        class Test(ValidationCase):
            def test(self):
                pass

        test = Test()
        with self.assertRaises(TestMissingResults):
            test.run()
        self.assertEqual(len(test.results), 0)

    def testAddFloatData(self):
        args = {'key': 'peak_temperature',
                'value': 1.234,
                'units': 'K',
                'description': 'Peak temperature'}

        test = ValidationCase()
        test.addFloatData(*args.values())
        all_data = test.data
        self.assertEqual(len(all_data), 1)
        data = test.data[args['key']]
        for key, value in args.items():
            self.assertEqual(getattr(data, key), value)
        self.assertIsNone(data.test)

    def testAddFloatDataNotFloat(self):
        test = ValidationCase()
        with self.assertRaises(ValueError):
            test.addFloatData('unused', int(1), None, 'unused')

    def testAddFloatDataBounded(self):
        key = 'data'
        value = 1.234
        bounds = (value - 1.0, value + 0.1)

        test = ValidationCase()
        test.addFloatData(key, value, None, 'unused', bounds=bounds)
        all_data = test.data
        self.assertEqual(len(all_data), 1)
        data = all_data[key]
        self.assertEqual(bounds, data.bounds)

    def testAddFloatDataNominal(self):
        key = 'test'
        nominal = 1.1
        test = ValidationCase()
        test.addFloatData(key, 1.0, None, 'unused', nominal=nominal)
        all_data = test.data
        self.assertEqual(len(all_data), 1)
        self.assertEqual(nominal, all_data[key].nominal)

    def testAddFloatDataBoundedCheck(self):
        class Test(ValidationCase):
            def test_pass(self):
                self.addFloatData('pass', 1.0, None, 'foo', bounds=(0.9, 1.1))
            def test_fail_lower(self):
                self.addFloatData('fail_lower', 2.0, None, 'foo', bounds=(2.1, 3.0))
            def test_fail_upper(self):
                self.addFloatData('fail_upper', 3.0, None, 'foo', bounds=(2.0, 2.2))

        test = Test()
        test.run()

        results = test.results
        self.assertEqual(len(results), 3)
        for case in ['pass', 'fail_lower', 'fail_upper']:
            filtered = [r for r in results if r.test.endswith(case)]
            self.assertEqual(len(filtered), 1)
            result = filtered[0]
            status = test.Status.OK if case == 'pass' else test.Status.FAIL
            self.assertEqual(status, result.status)
            self.assertEqual(case, result.data_key)
            data = test.data[case]
            self.assertEqual(f'Test.test_{case}', data.test)

    def testInitialize(self):
        class Test(ValidationCase):
            def __init__(self):
                self.value_set = False
                super().__init__()
            def initialize(self):
                self.value_set = True
            def test(self):
                self.addResult(self.Status.FAIL, 'foo')

        test = Test()
        self.assertFalse(test.value_set)
        test.run()
        self.assertTrue(test.value_set)

    def testFinalize(self):
        class Test(ValidationCase):
            def __init__(self):
                self.value_set = False
                super().__init__()
            def finalize(self):
                self.value_set = True
            def test(self):
                self.addResult(self.Status.FAIL, 'foo')

        test = Test()
        self.assertFalse(test.value_set)
        test.run()
        self.assertTrue(test.value_set)

    def testTestException(self):
        class Test(ValidationCase):
            def test(self):
                raise Exception('foo')

        test = Test()
        with self.assertRaises(TestRunException):
            test.run()

    def testGetTesterOutputs(self):
        outputs = ['foo.csv', 'bar.e']
        test = ValidationCase(tester_outputs = outputs)
        self.assertEqual(outputs, test.getTesterOutputs())
        self.assertEqual(['bar.e'], test.getTesterOutputs(extension='.e'))

    def testWithParameters(self):
        required_value = 500
        default_value = 1.0
        optional_value = 2.0

        params = InputParameters()
        params.addRequiredParam('required', 'A required value')
        params.addParam('default', default_value, 'A default value')
        params.addParam('optional', 'An optional value')

        params['required'] = required_value
        params['optional'] = optional_value

        class Test(ValidationCase):
            def initialize(self):
                self.required = self.getParam('required')
                self.default = self.getParam('default')
                self.optional = self.getParam('optional')

        test = Test(params=params)
        test.initialize()
        self.assertEqual(test.required, required_value)
        self.assertEqual(test.default, default_value)
        self.assertEqual(test.optional, optional_value)

if __name__ == '__main__':
    unittest.main()
