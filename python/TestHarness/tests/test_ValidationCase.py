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

    def testAddData(self):
        args = {'key': 'some_dict',
                'value': {'foo': 'bar'},
                'description': 'Useless dictionary'}
        test = ValidationCase()
        test.addData(*args.values())
        all_data = test.data
        self.assertEqual(len(all_data), 1)
        data = test.data[args['key']]
        for key, value in args.items():
            self.assertEqual(getattr(data, key), value)
        self.assertIsNone(data.test)

    def testAddDataCheckType(self):
        case = ValidationCase()

        # Description not a string
        with self.assertRaisesRegex(TypeError, 'description is not of type str'):
            case.addData('unused', None, None)

    def testAddDataNonSerializable(self):
        with self.assertRaisesRegex(TypeError, 'not JSON serializable'):
            ValidationCase().addData('key', b'1234', 'unused')

    def testAddScalarData(self):
        args = {'key': 'peak_temperature',
                'value': 1.234,
                'description': 'Peak temperature',
                'units': 'K'}

        test = ValidationCase()
        test.addScalarData(*args.values())
        all_data = test.data
        self.assertEqual(len(all_data), 1)
        data = test.data[args['key']]
        for key, value in args.items():
            self.assertEqual(getattr(data, key), value)
        self.assertIsNone(data.test)

    def testAddScalarDataCheckType(self):
        case = ValidationCase()

        # Non-numeric value
        with self.assertRaisesRegex(TypeError, 'value: not of type float or int'):
            ValidationCase().addScalarData('non_numeric', 'abcd', 'unused', None)

        # Allow integers
        value = int(1)
        case.addScalarData('to_float', value, 'unused', None)
        data = case.data['to_float']
        self.assertTrue(isinstance(data.value, float))
        self.assertEqual(data.value, float(value))

        # Non string description
        with self.assertRaisesRegex(TypeError, 'description: not of type str'):
            ValidationCase().addScalarData('bad_description', 1, None, None)

        # Non string units
        with self.assertRaisesRegex(TypeError, 'units: not of type str or None'):
            ValidationCase().addScalarData('bad_units', 1, 'unused', 1)

        # Non-tuple bounds
        with self.assertRaisesRegex(TypeError, 'bounds: not of type tuple'):
            ValidationCase().addScalarData('bad_bounds', 1, 'unused', None, bounds=[])

        # Bad-sized bounds
        with self.assertRaisesRegex(TypeError, 'bounds: not of length 2'):
            ValidationCase().addScalarData('bad_bounds_len', 1, 'unused', None, bounds=(None, None, None))

        # Bad min bounds type
        with self.assertRaisesRegex(TypeError, 'bounds: min bounds not of type int or float'):
            ValidationCase().addScalarData('bad_bounds_len', 1, 'unused', None, bounds=(None, None))

        # Bad max bounds type
        with self.assertRaisesRegex(TypeError, 'bounds: max bounds not of type int or float'):
            ValidationCase().addScalarData('bad_bounds_len', 1, 'unused', None, bounds=(1, None))

        # Bounds int to float
        case.addScalarData('bounds_to_float', 1, 'unused', None, bounds=(1, 2))
        bounds_to_float_data = case.data['bounds_to_float']
        assert isinstance(bounds_to_float_data.bounds[0], float)
        assert isinstance(bounds_to_float_data.bounds[1], float)

    def testAddScalarDataBounded(self):
        key = 'data'
        value = 1.234
        bounds = (value - 1.0, value + 0.1)

        test = ValidationCase()
        test.addScalarData(key, value, 'unused', None, bounds=bounds)
        all_data = test.data
        self.assertEqual(len(all_data), 1)
        data = all_data[key]
        self.assertEqual(bounds, data.bounds)

    def testAddScalarDataNominal(self):
        key = 'test'
        nominal = 1.1
        test = ValidationCase()
        test.addScalarData(key, 1.0, 'unused', None,  nominal=nominal)
        all_data = test.data
        self.assertEqual(len(all_data), 1)
        self.assertEqual(nominal, all_data[key].nominal)

    def testAddScalarDataBoundedCheck(self):
        class Test(ValidationCase):
            def test_pass(self):
                self.addScalarData('pass', 1.0, 'foo', None, bounds=(0.9, 1.1))
            def test_fail_lower(self):
                self.addScalarData('fail_lower', 2.0, 'foo', None, bounds=(2.1, 3.0))
            def test_fail_upper(self):
                self.addScalarData('fail_upper', 3.0, 'foo', None, bounds=(2.0, 2.2))

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
            if case == 'pass':
                self.assertIn('within bounds', result.message)
            else:
                self.assertIn('out of bounds', result.message)
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
