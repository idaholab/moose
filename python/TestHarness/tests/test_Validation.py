#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

import unittest
from copy import deepcopy
import json
import os

class TestValidation(TestHarnessTestCase):
    def compareGold(self, validation: dict, name: str, rewrite: bool = False):
        """
        Helper for comparing against a gold file, which contains the 'validation'
        entry for the given test.

        When running this, you can set rewrite=True to rewrite the gold.
        """
        gold_path = os.path.join('gold', 'validation', f'validation_{name}.json')

        # Take a copy as we'll change this
        validation = deepcopy(validation)

        # Rewrite the script path so that it is relative and we don't
        # diff based on where this is ran
        validation['script'] = os.path.relpath(validation['script'])

        if rewrite:
            with open(gold_path, 'w') as f:
                json.dump(validation, f, indent=2, sort_keys=True)

        with open(gold_path, 'r') as f:
            validation_gold = json.load(f)

        self.assertEqual(validation, validation_gold)

    def test(self):
        """
        Tests running a basic validation case with the TestHarness,
        using the `ok` test in the `validation` test spec
        """
        results = self.runTests('-i', 'validation', '--re', 'ok')
        out = results.results
        self.assertEqual(out['testharness']['validation_version'],
                         results.harness.VALIDATION_VERSION)

        test = out['tests']['tests/test_harness']['tests']['ok']
        status = test['status']
        self.assertEqual(status['status'], 'OK')

        # Validation entry
        validation = test['validation']

        # Compare against the golded values
        # If this fails, you can regold by setting rewrite = true in compareGold
        self.compareGold(validation, 'test')

        # Check validation output
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
        self.assertEqual('ValidationScalarData', number['type'])
        # Arbitrary data (dict)
        useless_dict = data['useless_dict']
        self.assertEqual({'foo': 'bar'}, useless_dict['value'])
        self.assertEqual('A useless dictionary', useless_dict['description'])
        self.assertEqual('ValidationData', useless_dict['type'])
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
        self.assertEqual('ValidationVectorData', vector['type'])

        # Check on-screen output
        output = test['output']
        validation_output = output['validation']
        self.assertIn('Running validation case', validation_output)
        self.assertIn('Acquired 3 data value(s), 3 result(s): 3 ok, 0 fail, 0 skip', validation_output)

        # Check timing; validation execution exists
        timing = test['timing']
        self.assertIn('validation_run', timing)

    def testCSV(self):
        """
        Tests running a basic CSV validation case with the TestHarness,
        using the `csv` test in the `validation` test spec.
        """
        results = self.runTests('-i', 'validation', '--re', 'csv')
        out = results.results
        harness = results.harness
        self.assertEqual(out['testharness']['validation_version'],
                         results.harness.VALIDATION_VERSION)

        test = out['tests']['tests/test_harness']['tests']['csv']
        status = test['status']
        self.assertEqual(status['status'], 'OK')

        # Validation entry
        validation = test['validation']
        # Compare against the golded values
        # If this fails, you can regold by setting rewrite = true in compareGold
        self.compareGold(validation, 'testcsv')

        # Make sure the output path shows up in output files
        job = harness.finished_jobs[0]
        csv = os.path.join(job.getTestDir(), job.validation_cases[0].getParam('validation_csv'))
        self.assertIn(csv, job.getOutputFiles(harness.options))

    def testFail(self):
        """
        Tests running a basic validation case with the TestHarness
        that fails, using the `fail` test in the `validation`
        test spec
        """
        out = self.runTests('-i', 'validation', '--re', 'fail', exit_code=132).results
        test = out['tests']['tests/test_harness']['tests']['fail']
        status = test['status']
        self.assertEqual(status['status'], 'ERROR')
        self.assertEqual(status['status_message'], 'VALIDATION FAILED')

        validation = test['validation']

        # Compare against the golded values
        # If this fails, you can regold by setting rewrite = true in compareGold
        self.compareGold(validation, 'testfail')

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
        """
        Tests running a basic validation case with the TestHarness
        that throws a python exception, using the `exception` test
        in the `validation` test spec
        """
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

    def testInitException(self):
        """
        Tests a validation case initialization failure using the
        `validation_init_exception` test spec
        """
        out = self.runTests('-i', 'validation_init_exception', exit_code=132).results
        test = out['tests']['tests/test_harness']['tests']['test']

        # Should have failed
        status = test['status']
        self.assertEqual(status['status'], 'ERROR')
        self.assertEqual(status['status_message'], 'VALIDATION INIT EXCEPTION')

        # Check that the exception is in on screen output
        output = test['output']['job']
        self.assertIn("Python exception encountered in validation case", output)
        self.assertIn("validation_init_exception.py", output)
        self.assertIn("raise Exception('foo')", output)

    def testBadPython(self):
        """
        Tests running a validation case with the TestHarness that
        has invalid python syntax, using the `validation_bad_python`
        test spec
        """
        out = self.runTests('-i', 'validation_bad_python', exit_code=128).output
        self.assertIn('validation_bad_python:2:   invalid syntax (validation_badpython.py, line 1)', out)

    def testDuplicateParam(self):
        """
        Tests a validation case that specifies the same parameter
        multiple times from different cases
        """
        out = self.runTests('-i', 'validation_duplicate_param', exit_code=128).output
        self.assertIn('validation_duplicate_param:2: Failed to create Tester: Duplicate parameter "type" from validation test', out)

if __name__ == '__main__':
    unittest.main()
