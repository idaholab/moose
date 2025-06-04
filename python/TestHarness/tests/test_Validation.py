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
        validation = test['validation']
        results = validation['results']
        self.assertEqual(len(results), 1)
        result = results[0]
        self.assertEqual('OK', result['status'])
        self.assertIn('within bounds', result['message'])
        self.assertEqual('TestCase.testValidation', result['test'],)
        self.assertEqual('number', result['data_key'])
        self.assertTrue(result['script'].endswith('validation_ok.py'))
        data = validation['data']
        self.assertEqual(len(data), 1)
        number = data['number']
        self.assertEqual(100.0, number['value'])
        self.assertEqual('Number', number['description'])
        self.assertEqual('coolunits', number['units'])
        self.assertEqual(95.0, number['bounds'][0])
        self.assertEqual(105.0, number['bounds'][1])
        self.assertTrue(number['script'].endswith('validation_ok.py'))
        output = test['output']
        validation_output = output['validation']
        self.assertIn('Running validation case', validation_output)
        self.assertIn('Acquired 1 data value(s), 1 result(s): 1 ok, 0 fail, 0 skip', validation_output)
        timing = test['timing']
        self.assertIn('validation_init', timing)
        self.assertIn('validation_run', timing)

    def testFail(self):
        out = self.runTests('-i', 'validation', '--re', 'fail').results
        test = out['tests']['tests/test_harness']['tests']['fail']
        status = test['status']
        self.assertEqual(status['status'], 'ERROR')
        self.assertEqual(status['status_message'], 'VALIDATION FAILED')
        validation = test['validation']
        results = validation['results']
        self.assertEqual(len(results), 1)
        result = results[0]
        self.assertEqual('FAIL', result['status'])
        self.assertIn('out of bounds', result['message'])
        self.assertEqual('TestCase.testValidation', result['test'],)
        self.assertEqual('number', result['data_key'])
        data = validation['data']
        self.assertEqual(len(data), 1)
        number = data['number']
        self.assertEqual(100, int(number['value']))
        self.assertEqual('Number', number['description'])
        self.assertEqual(None, number['units'])
        self.assertEqual(101.0, number['bounds'][0])
        self.assertEqual(200.0, number['bounds'][1])
        output = test['output']
        validation_output = output['validation']
        self.assertIn('Running validation case', validation_output)
        self.assertIn('Acquired 1 data value(s), 1 result(s): 0 ok, 1 fail, 0 skip', validation_output)
        timing = test['timing']
        self.assertIn('validation_init', timing)
        self.assertIn('validation_run', timing)

    def testException(self):
        out = self.runTests('-i', 'validation', '--re', 'exception').results
        test = out['tests']['tests/test_harness']['tests']['exception']
        status = test['status']
        self.assertEqual(status['status'], 'ERROR')
        self.assertEqual(status['status_message'], 'VALIDATION TEST EXCEPTION')
        validation = test['validation']
        self.assertEqual(0, len(validation['results']))
        self.assertEqual(0, len(validation['data']))
        output = test['output']
        validation_output = output['validation']
        self.assertIn('Exception: foo', validation_output)
        self.assertIn('Encountered exception(s) while running tests', validation_output)

    def testBadPython(self):
        out = self.runTests('-i', 'validation_bad_python').output
        self.assertIn('validation_bad_python:   invalid syntax (validation_badpython.py, line 1)', out)
