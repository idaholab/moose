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
import numpy as np
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

    def testCheckBounds(self):
        number_format = ValidationCase.number_format
        value, min_value, max_value, units = 1, 0, 2, 'coolunits'

        # Success
        status, message = ValidationCase.checkBounds(value, min_value, max_value, 'coolunits')
        self.assertEqual(status, ValidationCase.Status.OK)
        exp_message = [f'value {value:{number_format}} {units}']
        exp_message += ['within bounds;']
        exp_message += [f'min = {min_value:{number_format}} {units},']
        exp_message += [f'max = {max_value:{number_format}} {units}']
        self.assertEqual(message, ' '.join(exp_message))

        # Success no units
        status, message = ValidationCase.checkBounds(value, min_value, max_value, None)
        self.assertEqual(status, ValidationCase.Status.OK)
        exp_message = [f'value {value:{number_format}}']
        exp_message += ['within bounds;']
        exp_message += [f'min = {min_value:{number_format}},']
        exp_message += [f'max = {max_value:{number_format}}']
        self.assertEqual(message, ' '.join(exp_message))

        # Fail lower
        status, message = ValidationCase.checkBounds(value, 1.5, max_value, None)
        self.assertEqual(status, ValidationCase.Status.FAIL)
        exp_message = [f'value {value:{number_format}}']
        exp_message += ['out of bounds;']
        exp_message += [f'min = {1.5:{number_format}},']
        exp_message += [f'max = {max_value:{number_format}}']
        self.assertEqual(message, ' '.join(exp_message))

        # Fail upper
        status, message = ValidationCase.checkBounds(value, min_value, 0.5, None)
        self.assertEqual(status, ValidationCase.Status.FAIL)
        exp_message = [f'value {value:{number_format}}']
        exp_message += ['out of bounds;']
        exp_message += [f'min = {min_value:{number_format}},']
        exp_message += [f'max = {0.5:{number_format}}']
        self.assertEqual(message, ' '.join(exp_message))

    def testCheckBoundsChecks(self):
        # Non-numeric values
        with self.assertRaisesRegex(ValueError, 'value: could not convert string'):
            ValidationCase.checkBounds('abc', 0, 1, None)
        with self.assertRaisesRegex(ValueError, 'min_bound: could not convert string'):
            ValidationCase.checkBounds(0, 'abc', 1, None)
        with self.assertRaisesRegex(ValueError, 'max_bound: could not convert string'):
            ValidationCase.checkBounds(0, 1, 'abc', None)

        # Bound min greater than max
        with self.assertRaisesRegex(ValueError, 'min_bound greater than max_bound'):
            ValidationCase.checkBounds(0, 2, 1, None)

    def testToFloat(self):
        # Success
        value = float(1.01)
        self.assertEqual(value, ValidationCase.toFloat(value))

        # Integer to float
        value = int(1)
        to_value = ValidationCase.toFloat(value)
        self.assertTrue(isinstance(to_value, float))
        self.assertEqual(to_value, float(value))

        # Catch exception
        context = 'some context'
        with self.assertRaisesRegex(ValueError, 'some context: could not convert string to float'):
            ValidationCase.toFloat('abcd', context)

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

    def testAddScalarDataChecks(self):
        case = ValidationCase()

        # Non-numeric value
        with self.assertRaisesRegex(ValueError, 'value: could not convert string to float'):
            ValidationCase().addScalarData('k', 'abcd', 'unused', None)

        # Allow integers
        value = int(1)
        case.addScalarData('to_float', value, 'unused', None)
        data = case.data['to_float']
        self.assertTrue(isinstance(data.value, float))
        self.assertEqual(data.value, float(value))

        # Non string description
        with self.assertRaisesRegex(TypeError, 'description: not of type str'):
            ValidationCase().addScalarData('k', 1, None, None)

        # Non string units
        with self.assertRaisesRegex(TypeError, 'units: not of type str or None'):
            ValidationCase().addScalarData('k', 1, 'unused', 1)

        # Non-tuple bounds
        with self.assertRaisesRegex(TypeError, 'bounds: not of type tuple'):
            ValidationCase().addScalarData('k', 1, 'unused', None, bounds=[])

        # Bad-sized bounds
        with self.assertRaisesRegex(TypeError, 'bounds: not of length 2'):
            ValidationCase().addScalarData('k', 1, 'unused', None, bounds=(None, None, None))

        # Bad min bounds type
        with self.assertRaisesRegex(TypeError, 'bounds min: float\\(\\) argument must be a string or a real number'):
            ValidationCase().addScalarData('k', 1, 'unused', None, bounds=(None, None))

        # Bad max bounds type
        with self.assertRaisesRegex(ValueError, 'bounds max: could not convert string to float'):
            ValidationCase().addScalarData('k', 1, 'unused', None, bounds=(1, 'abcd'))

        # Bounds int to float
        case.addScalarData('bounds_to_float', 1, 'unused', None, bounds=(1, 2))
        bounds_to_float_data = case.data['bounds_to_float']
        assert isinstance(bounds_to_float_data.bounds[0], float)
        assert isinstance(bounds_to_float_data.bounds[1], float)

        # Non-numeric nominal
        with self.assertRaisesRegex(ValueError, 'nominal: could not convert string to float'):
            ValidationCase().addScalarData('k', 1, 'unused', None, nominal='abcd')

        # Int nominal to float
        nominal = int(1)
        case.addScalarData('nominal_to_float', 1, 'unused', None, nominal=nominal)
        data = case.data['nominal_to_float']
        self.assertTrue(isinstance(data.nominal, float))
        self.assertEqual(data.nominal, float(nominal))

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

    def testToListFloat(self):
        # Success list
        values = [1.0, 2.0]
        self.assertEqual(ValidationCase.toListFloat(values), values)

        # Success numpy array
        values = [1.0, 2.0]
        self.assertEqual(ValidationCase.toListFloat(np.array(values)), values)

        # Success int to float
        values = [int(1), int(2)]
        to_values = ValidationCase.toListFloat(values)
        self.assertTrue(isinstance(to_values[0], float))
        self.assertTrue(isinstance(to_values[1], float))
        self.assertEqual(to_values, [float(values[0]), float(values[1])])

        # Failed numpy array
        values = ['abcd']
        with self.assertRaisesRegex(TypeError, 'array conversion failed'):
            ValidationCase.toListFloat(values)
        with self.assertRaisesRegex(TypeError, 'foobar: array conversion failed'):
            ValidationCase.toListFloat(values, 'foobar:')

        # Not one-dimensional
        values = [[1], [1]]
        with self.assertRaisesRegex(TypeError, 'not one-dimensional'):
            ValidationCase.toListFloat(values)
        with self.assertRaisesRegex(TypeError, 'foobar: not one-dimensional'):
            ValidationCase.toListFloat(values, 'foobar:')

        # Has nans
        values = [None]
        with self.assertRaisesRegex(ValueError, 'value at index 0 is nan'):
            ValidationCase.toListFloat(values)
        with self.assertRaisesRegex(ValueError, 'foobar: value at index 0 is nan'):
            ValidationCase.toListFloat(values, 'foobar:')

    def testAddVectorDataChecks(self):
        good_args = {'x': ([0.0, 1.0], 'description_x', 'units_x'),
                     'value': ([1.0, 2.0], 'description_value', 'units_value')}

        for key in ['x', 'value']:
            # Non-tuple entry
            args = dict(good_args)
            args[key] = None
            with self.assertRaisesRegex(TypeError, f'{key}: not a tuple'):
                ValidationCase().addVectorData('k', *args.values())
            # Bad-length tuple
            args = dict(good_args)
            args[key] = (None, None)
            with self.assertRaisesRegex(TypeError, f'{key}: not of length 3.*'):
                ValidationCase().addVectorData('k', *args.values())

            # Non-1D data
            args = dict(good_args)
            args[key] = (np.array([[0]]), 'unused', None)
            with self.assertRaisesRegex(TypeError, f'{key}: first entry \\(values\\) not one-dimensional'):
                ValidationCase().addVectorData('k', *args.values())
            # To-array failed
            args = dict(good_args)
            args[key] = ([1., 'abcd'], 'unused', None)
            with self.assertRaisesRegex(TypeError, f'{key}: first entry \\(values\\) array conversion failed'):
                ValidationCase().addVectorData('k', *args.values())
            # Cast int list values to floats
            args = dict(good_args)
            args[key] = ([1., 1], 'description', 'units')
            case = ValidationCase()
            data_key = f'{key}_cast_int'
            case.addVectorData(data_key, *args.values())
            data = getattr(case.data[data_key], key)
            for v in data:
                self.assertTrue(isinstance(v, float))

            # Non-string description
            args = dict(good_args)
            args[key] = ([], None, None)
            with self.assertRaisesRegex(TypeError, f'{key}: second entry \\(description\\) not of type str'):
                ValidationCase().addVectorData('k', *args.values())

            # Non-string units
            args = dict(good_args)
            args[key] = ([], 'desc', 1)
            with self.assertRaisesRegex(TypeError, f'{key}: third entry \\(units\\) is not of type str or None'):
                ValidationCase().addVectorData('k', *args.values())

            # Value lengths inconsistent
            args[key] = ([0.0], 'desc', None)
            with self.assertRaisesRegex(ValueError, 'Length of x and value values not the same'):
                ValidationCase().addVectorData('k', *args.values())

        # Non-tuple bounds
        with self.assertRaisesRegex(TypeError, 'bounds: not of type tuple'):
            ValidationCase().addVectorData('k', *good_args.values(), bounds='foo')
        # Bad-sized bounds
        with self.assertRaisesRegex(TypeError, 'bounds: not of length 2'):
            ValidationCase().addVectorData('k', *good_args.values(), bounds=(None, None, None))
        # Bounds not same length as data
        with self.assertRaisesRegex(ValueError, 'bounds: min not same length as data'):
            ValidationCase().addVectorData('k', *good_args.values(),
                                           bounds=([0.0], [0.0, 1.0]))
        with self.assertRaisesRegex(ValueError, 'bounds: max not same length as data'):
            ValidationCase().addVectorData('k', *good_args.values(),
                                           bounds=([0.0, 1.0], [0.0]))

        # Bad array for nominal
        with self.assertRaisesRegex(TypeError, 'nominal: array conversion failed'):
            ValidationCase().addVectorData('k', *good_args.values(), nominal=['abc'])
        # Bad length for nominal
        with self.assertRaisesRegex(TypeError, 'nominal: not same length as data'):
            ValidationCase().addVectorData('k', *good_args.values(), nominal=[1, 2, 3])

    def testAddVectorData(self):
        x = ([0.0, 1.0], 'Position', 'cm')
        value = ([1.0, 2.0], 'Temperature', 'K')

        test = ValidationCase()
        data_key = 'temperature'
        test.addVectorData('temperature', x, value)

        self.assertEqual(len(test.data), 1)
        data = test.data[data_key]
        self.assertEqual(data.key, data_key)
        self.assertTrue(data.validation)
        self.assertEqual(data.value, value[0])
        self.assertEqual(data.description, value[1])
        self.assertEqual(data.units, value[2])
        self.assertEqual(data.x, x[0])
        self.assertEqual(data.x_description, x[1])
        self.assertEqual(data.x_units, x[2])

    def testAddVectorDataNominal(self):
        key = 'k'
        x = ([0.0, 1.0], 'Position', 'cm')
        value = ([1.0, 2.0], 'Temperature', 'K')
        nominal = [2.0, 3.0]

        test = ValidationCase()
        test.addVectorData(key, x, value, nominal=nominal)
        all_data = test.data
        self.assertEqual(len(all_data), 1)
        self.assertEqual(nominal, all_data[key].nominal)

    def testAddVectorDataBounded(self):
        key = 'data'
        x = ([0, 1], 'description_x', 'units_x')
        value = ([1, 2], 'description_value', 'units_value')
        bounds = ([0, 1], [2, 3])

        test = ValidationCase()
        test.addVectorData(key, x, value, bounds=bounds)
        self.assertEqual(len(test.data), 1)
        self.assertEqual(bounds, test.data[key].bounds)

    def testAddVectorDataBoundedCheck(self):
        class TestValidationCase(ValidationCase):
            def test_pass(self):
                self.addVectorData('pass',
                                   ([0, 1], 'x', 'x_units'),
                                   ([1, 2], 'y', 'y_units'),
                                   bounds=([0.9, 1.9], [1.1, 2.1]))
            def test_fail_lower(self):
                self.addVectorData('fail_lower',
                                   ([0, 1], 'x', 'x_units'),
                                   ([1, 2], 'y', 'y_units'),
                                   bounds=([1.05, 2.05], [1.1, 2.1]))
            def test_fail_upper(self):
                self.addVectorData('fail_upper',
                                   ([0, 1], 'x', 'x_units'),
                                   ([1, 2], 'y', 'y_units'),
                                   bounds=([0.9, 1.9], [0.95, 1.95]))
            def test_fail_both(self):
                self.addVectorData('fail_both',
                                   ([0, 1], 'x', 'x_units'),
                                   ([1, 2], 'y', 'y_units'),
                                   bounds=([1.05, 1.9], [1.1, 1.95]))

        case = TestValidationCase()
        case.run()
        self.assertEqual(len(case.results), 8)

        def check_result(test, index, status):
            results = [r for r in case.results if (r.test.endswith(test) and f'(index {index})' in r.message)]
            self.assertEqual(len(results), 1)
            result = results[0]
            data = case.data[test]
            x = data.x[index]
            self.assertEqual(result.status, status)
            self.assertTrue(result.message.startswith(f'x = {x:{ValidationCase.number_format}}'))

        for i in range(2):
            check_result('pass', i, ValidationCase.Status.OK)
            check_result('fail_lower', i, ValidationCase.Status.FAIL)
            check_result('fail_upper', i, ValidationCase.Status.FAIL)
            check_result('fail_both', i, ValidationCase.Status.FAIL)

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
