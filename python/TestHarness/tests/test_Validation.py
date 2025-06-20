#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase
class TestHarnessTester(TestHarnessTestCase):
    def test(self):
        out = self.runTests('-i', 'validation', '--re', 'ok').results
        test = out['tests']['tests/test_harness']['tests']['ok']
        status = test['status']
        self.assertEqual(status['status'], 'OK')

        # Check validation output
        validation = test['validation']
        self.assertTrue(validation['script'].endswith('validation_ok.py'))
        # Validation results
        results = validation['results']
        self.assertEqual(len(results), 3)
        # Results from number
        number_results = [r for r in results if r['data_key'] == 'number']
        self.assertEqual(len(number_results), 1)
        number_result = number_results[0]
        self.assertEqual('OK', number_result['status'])
        self.assertIn('within bounds', number_result['message'])
        self.assertEqual('TestCase.testValidation', number_result['test'])
        # Results from vector
        vector_results = [r for r in results if r['data_key'] == 'vector']
        self.assertEqual(len(vector_results), 2)
        for i, result in enumerate(vector_results):
            self.assertEqual('OK', result['status'])
            self.assertIn('within bounds', result['message'])
            self.assertIn(f'index {i}', result['message'])
            self.assertEqual('TestCase.testValidation', result['test'])

        # Validation data
        data = validation['data']
        self.assertEqual(len(data), 3)
        # Numeric value
        number = data['number']
        self.assertEqual(100.0, number['value'])
        self.assertEqual('Number', number['description'])
        self.assertEqual('coolunits', number['units'])
        self.assertEqual(95.0, number['bounds'][0])
        self.assertEqual(105.0, number['bounds'][1])
        self.assertEqual('ScalarData', number['type'])
        # Arbitrary data (dict)
        useless_dict = data['useless_dict']
        self.assertEqual({'foo': 'bar'}, useless_dict['value'])
        self.assertEqual('A useless dictionary', useless_dict['description'])
        self.assertEqual('Data', useless_dict['type'])
        # Vector data
        vector = data['vector']
        self.assertEqual([0, 1], vector['x'])
        self.assertEqual([1, 2], vector['value'])
        self.assertEqual('Position', vector['x_description'])
        self.assertEqual('Temperature', vector['description'])
        self.assertEqual('cm', vector['x_units'])
        self.assertEqual('K', vector['units'])
        self.assertEqual([0, 1], vector['bounds'][0])
        self.assertEqual([2, 3], vector['bounds'][1])
        self.assertEqual('VectorData', vector['type'])

        # Check on-screen output
        output = test['output']
        validation_output = output['validation']
        self.assertIn('Running validation case', validation_output)
        self.assertIn('Acquired 3 data value(s), 3 result(s): 3 ok, 0 fail, 0 skip', validation_output)

        # Check timing; validation execution exists
        timing = test['timing']
        self.assertIn('validation_init', timing)
        self.assertIn('validation_run', timing)

    def testFail(self):
        out = self.runTests('-i', 'validation', '--re', 'fail', exit_code=132).results
        test = out['tests']['tests/test_harness']['tests']['fail']
        status = test['status']
        self.assertEqual(status['status'], 'ERROR')
        self.assertEqual(status['status_message'], 'VALIDATION FAILED')

        # Check validation output
        validation = test['validation']
        # Validation results (one should have failed)
        results = validation['results']
        self.assertEqual(len(results), 1)
        result = results[0]
        self.assertEqual('FAIL', result['status'])
        self.assertIn('out of bounds', result['message'])
        self.assertEqual('TestCase.testValidation', result['test'],)
        self.assertEqual('number', result['data_key'])
        # Validation data
        data = validation['data']
        self.assertEqual(len(data), 1)
        number = data['number']
        self.assertEqual(100, int(number['value']))
        self.assertEqual('Number', number['description'])
        self.assertIsNone(number['units'])
        self.assertEqual(101.0, number['bounds'][0])
        self.assertEqual(200.0, number['bounds'][1])

        # Check on-screen output
        output = test['output']
        validation_output = output['validation']
        self.assertIn('Running validation case', validation_output)
        self.assertIn('Acquired 1 data value(s), 1 result(s): 0 ok, 1 fail, 0 skip', validation_output)

    def testException(self):
        out = self.runTests('-i', 'validation', '--re', 'exception', exit_code=132).results
        test = out['tests']['tests/test_harness']['tests']['exception']
        status = test['status']
        self.assertEqual(status['status'], 'ERROR')
        self.assertEqual(status['status_message'], 'VALIDATION TEST EXCEPTION')

        # Check validation output; shouldn't have anything
        validation = test['validation']
        self.assertEqual(0, len(validation['results']))
        self.assertEqual(0, len(validation['data']))

        # Check on screen output; found an exception
        output = test['output']
        validation_output = output['validation']
        self.assertIn('Exception: foo', validation_output)
        self.assertIn('Encountered exception(s) while running tests', validation_output)

    def testBadPython(self):
        out = self.runTests('-i', 'validation_bad_python', exit_code=128).output
        self.assertIn('validation_bad_python:   invalid syntax (validation_badpython.py, line 1)', out)

    def testDuplicateParam(self):
        out = self.runTests('-i', 'validation_duplicate_param', exit_code=128).output
        self.assertIn('Duplicate parameter "type" from validation test', out)
