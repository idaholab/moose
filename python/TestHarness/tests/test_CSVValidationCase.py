#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: disable
from copy import deepcopy
from dataclasses import asdict
import os
import json
import unittest
from tempfile import TemporaryDirectory

import pandas as pd
from TestHarness.validation import CSVValidationCase

DEFAULT_REL_ERR: float = float(CSVValidationCase.validParams()['validation_rel_err'])

class TestCSVValidationCase(unittest.TestCase):
    maxDiff = 2000

    def compareGold(self, case: CSVValidationCase, name: str, rewrite: bool = False):
        """
        Helper for comparing against a gold file.

        When running this, you can set rewrite=True to rewrite the gold.
        """
        gold_path = os.path.join('gold', 'validation', f'csvvalidationcase_{name}.json')

        data = {'results': [asdict(v) for v in case.results],
                'data': {k: asdict(v) for k, v in case.data.items()}}
        data_dumped = json.dumps(data, indent=2, sort_keys=True)
        data_loaded = json.loads(data_dumped)

        if rewrite:
            with open(gold_path, 'w') as f:
                f.write(data_dumped)

        with open(gold_path, 'r') as f:
            gold_data = json.load(f)

        self.assertEqual(data_loaded, gold_data)

    def runValidationCase(self, gold_data, data, case_type, action=None,
                          set_params = {}, skip_write=False) -> CSVValidationCase:
        gold_file = 'gold.csv'

        params = CSVValidationCase.validParams()
        params['validation_csv'] = gold_file
        for k, v in set_params.items():
            params[k] = v

        gold_df = pd.DataFrame(gold_data)
        df = pd.DataFrame(data)

        cwd = os.getcwd()
        try:
            with TemporaryDirectory() as tmp_dir:
                os.chdir(tmp_dir)
                os.mkdir('gold')
                gold_df.to_csv(os.path.join('gold', gold_file), index=False)
                case = case_type(params)

                if not skip_write:
                    df.to_csv(gold_file, index=False)

                if not action:
                    case.run()
                else:
                    action(case)
        finally:
            os.chdir(cwd)

        return case

    def testInit(self):
        """
        Tests the error checking in __init__
        """
        # validation_csv is not in the directory
        out_of_dir_csv = '/abs/path'
        out_of_dir_params = CSVValidationCase.validParams()
        out_of_dir_params['validation_csv'] = '/abs/path'
        with self.assertRaisesRegex(ValueError, f'validation_csv={out_of_dir_csv} is not in this directory'):
            CSVValidationCase(out_of_dir_params)

        # Gold file is not found
        gold_not_found_params = CSVValidationCase.validParams()
        gold_not_found_params['validation_csv'] = 'foobar.csv'
        with self.assertRaisesRegex(FileNotFoundError, 'Gold CSV file'):
            CSVValidationCase(gold_not_found_params)

    def testInitialize(self):
        """
        Tests the error checking in __initialize__
        """
        # Can't find data file
        key = 'data'
        data = {'time': [0, 1],
                key: [0.0, 1.0]}
        class TestCase(CSVValidationCase):
            def test(self): pass
        with self.assertRaisesRegex(FileNotFoundError, r'CSV file (.*).csv not found'):
            self.runValidationCase(data, data, TestCase, skip_write=True)

    def testGetScalarCSV(self):
        """
        Tests the internal method _getScalarCSV for getting CSV
        values from either the gold or non-gold file
        """
        key = 'data_key'
        gold_data = {'time': [0, 1],
                     key: [1.0, 2.0]}
        data = {'time': [0, 1],
                key: [3.0, 4.0]}

        # Test passing for both files
        def get_value(case, gold, index):
            case.initialize()
            case_data = gold_data if gold else data
            value = case._getScalarCSV(key, index, gold)
            self.assertEqual(value, case_data[key][index])
        for index in [*range(len(data[key]))] + [-1]:
            for gold in [True, False]:
                self.runValidationCase(gold_data, data, CSVValidationCase, action=lambda case: get_value(case, gold, index))

        # Test out of range access for both files
        def out_of_range(case, gold):
            case.initialize()
            index = len(data[key])
            file = case._gold_csv if gold else case._csv
            with self.assertRaisesRegex(IndexError, f'Index {index} out of range in {file}'):
                case._getScalarCSV(key, len(data[key]), gold)
        for gold in [True, False]:
            self.runValidationCase(gold_data, data, CSVValidationCase, action=lambda case: out_of_range(case, gold))

        # Test missing key for both files
        def missing_ley(case, gold):
            missing_key = f'{key}_missing'
            case.initialize()
            file = case._gold_csv if gold else case._csv
            with self.assertRaisesRegex(KeyError, f'Column {missing_key} does not exist in {file}'):
                case._getScalarCSV(missing_key, -1, gold)
        for gold in [True, False]:
            self.runValidationCase(gold_data, data, CSVValidationCase, action=lambda case: missing_ley(case, gold))

        # Test overridding initialize() and not calling the base (where data is loaded)
        class BadInitialize(CSVValidationCase):
            def initialize(self):
                pass
        def bad_initialize(case):
            case.initialize()
            case._getScalarCSV(key, -1, False)
        with self.assertRaisesRegex(Exception, r'CSVValidationCase.initialize\(\) was not called in BadInitialize'):
            self.runValidationCase(gold_data, data, BadInitialize, action=bad_initialize)

    def testAddScalarCSVData(self):
        """
        Tests calling addScalarCSVData with a passing result
        """
        key, description, units = 'data', 'data_description', 'data_units'
        data = {'time': [0, 1],
                key: [0.0, 1.0]}
        value = data[key][-1]

        class TestCase(CSVValidationCase):
            def test(self):
                self.addScalarCSVData(key, -1, description, units)

        # Run where the gold and the known value are exactly the same
        case = self.runValidationCase(data, data, TestCase)

        # Check passed result
        self.assertEqual(len(case.results), 1)
        case_result = case.results[0]
        self.assertEqual(case_result.status, case.Status.OK)
        self.assertEqual(case_result.data_key, key)
        # Check data
        self.assertEqual(len(case.data), 1)
        case_data = case.data[key]
        self.assertEqual(case_data.key, key)
        self.assertEqual(case_data.value, value)
        self.assertEqual(case_data.description, description)
        self.assertEqual(case_data.units, units)
        self.assertEqual(case_data.nominal, value)
        self.assertEqual(case_data.rel_err, DEFAULT_REL_ERR)

        self.compareGold(case, 'testaddscalarcsvdata')

    def testAddScalarCSVDataFailRelErr(self):
        """
        Tests calling addScalarCSVData with a failed result on relative error
        """
        key, description, units = 'data', 'data_description', 'data_units'
        gold_data = {'time': [0, 1],
                     key: [0.0, 1.0]}
        data = deepcopy(gold_data)
        data[key][-1] = data[key][-1] + 0.01
        gold_value, value = gold_data[key][-1], data[key][-1]
        self.assertNotEqual(gold_value, value)

        class TestCase(CSVValidationCase):
            def test(self):
                self.addScalarCSVData(key, -1, description, units)

        rel_err = 1.e-4
        case = self.runValidationCase(gold_data, data, TestCase,
                                      set_params={'validation_rel_err': rel_err})
        self.assertEqual(case._rel_err, rel_err)

        # Checked passed result
        self.assertEqual(len(case.results), 1)
        case_result = case.results[0]
        self.assertEqual(case_result.status, case.Status.FAIL)
        # Check data
        self.assertEqual(len(case.data), 1)
        case_data = case.data[key]
        self.assertEqual(case_data.value, value)
        self.assertEqual(case_data.nominal, gold_value)
        self.compareGold(case, 'testaddscalarcsvdatafailrelerr')

        # Run the same thing, but with checking disabled
        class UncheckedTestCase(CSVValidationCase):
            def test(self):
                self.addScalarCSVData(key, -1, description, units, check=False)
                self.addResult(self.Status.OK, 'unused')
        unchecked_case = self.runValidationCase(gold_data, data, UncheckedTestCase)
        self.assertEqual(len(unchecked_case.results), 1)
        self.assertIsNone(unchecked_case.results[0].data_key)
        self.compareGold(case, 'testaddscalarcsvdatafailrelerrunchecked')

    def testAddScalarCSVDataStoreKey(self):
        """
        Tests calling addScalarCSVData with a 'store_key' argument,
        which stores the data with a different key
        """
        key = 'data'
        store_key = 'other'
        data = {'time': [0, 1],
                key: [0.0, 1.0]}

        class TestCase(CSVValidationCase):
            def test(self):
                self.addScalarCSVData(key, -1, 'unused_description', None, store_key=store_key)

        # Run where the gold and the known value are exactly the same
        case = self.runValidationCase(data, data, TestCase)

        # Data should be stored with a different key
        self.assertEqual(len(case.data), 1)
        self.assertIn(store_key, case.data)
        self.assertEqual(case.data[store_key].key, store_key)

        self.compareGold(case, 'testaddscalarcsvdatastorekey')

    def testAddScalarCSVDataAbsZero(self):
        """
        Tests calling addScalarCSVData with an absolute zero
        from a parameter
        """
        key = 'data'
        gold_data = {'time': [0], key: [0.0]}
        data = deepcopy(gold_data)
        data[key][0] = data[key][0] + 0.09
        gold_value, value = gold_data[key][-1], data[key][-1]
        self.assertNotEqual(gold_value, value)

        abs_zero = 0.1
        class TestCase(CSVValidationCase):
            def test(self):
                self.addScalarCSVData(key, -1, 'description', None)

        # Run the case where it should pass due to absolute zero
        abs_zero = 0.1
        pass_case = self.runValidationCase(gold_data, data, TestCase,
                                      set_params={'validation_abs_zero': abs_zero})
        # Check passed result, which passed due to abs zero
        self.assertEqual(len(pass_case.results), 1)
        case_result = pass_case.results[0]
        self.assertEqual(case_result.status, pass_case.Status.OK)
        self.assertIn('skipped due to absolute zero', case_result.message)
        # Check data abs_zero, which is overridden
        self.assertEqual(len(pass_case.data), 1)
        case_data = pass_case.data[key]
        self.assertEqual(case_data.rel_err, DEFAULT_REL_ERR)
        self.assertEqual(case_data.abs_zero, abs_zero)
        # Compare against gold
        self.compareGold(pass_case, 'testaddscalarcsvdataabszeropass')

        # Should not pass without abs_zero being set
        fail_case = self.runValidationCase(gold_data, data, TestCase)
        # Check failed result
        self.assertEqual(len(fail_case.results), 1)
        case_result = fail_case.results[0]
        self.assertEqual(case_result.status, fail_case.Status.FAIL)
        # Check data abs_zero, not overridden
        self.assertEqual(len(fail_case.data), 1)
        case_data = fail_case.data[key]
        self.assertEqual(case_data.rel_err, DEFAULT_REL_ERR)
        self.assertEqual(case_data.abs_zero, CSVValidationCase.DEFAULT_ABS_ZERO)
        # Compare against gold
        self.compareGold(pass_case, 'testaddscalarcsvdataabszerofail')

if __name__ == '__main__':
    unittest.main()
