#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import pandas
import numpy
try:
    from postprocessing import combine_csv
except ModuleNotFoundError:
    pass

class TestCombineCSV(unittest.TestCase):
    """
    Test use of combine_csv.py for combining csv files.
    """

    def setUp(self):
        """
        Define the pattern for test files.
        """
        self.__goldpath = os.path.abspath('../../test_files/gold')
        self.__basename = os.path.abspath('../../test_files/test_combine_in_')

    def tearDown(self):
        """
        Remove made CSV files
        """
        if os.path.exists("remove_me_54.csv"):
            os.remove("remove_me_54.csv")

    def testBasic(self):
        """
        Test basic usage with minimal options and headers written.
        """
        df_test = combine_csv.CombineCSV(self.__basename, "remove_me_54.csv",
                "large_number", write_header=True)
        self.assertTrue(df_test._ended)
        gold_df = pandas.read_csv("{}/combine_basic.csv".format(
            self.__goldpath))
        self.assertTrue(df_test._final_df.equals(gold_df),
            msg="Pandas dataframe is different from gold CSV for basic usage.")

    def testBasicTime(self):
        """
        Test basic usage with headers and a time file.
        """
        df_test = combine_csv.CombineCSV(self.__basename, "remove_me_54.csv",
                "large_number", write_header=True, timefile=True)
        self.assertTrue(df_test._ended)
        gold_df = pandas.read_csv("{}/combine_basic_time.csv".format(
            self.__goldpath))
        self.assertTrue(df_test._final_df.equals(gold_df),
            msg="Pandas dataframe is different from gold CSV for time usage.")

    def testBasicX(self):
        """
        Test basic usage with headers and a "x" variable name.
        """
        df_test = combine_csv.CombineCSV(self.__basename, "remove_me_54.csv",
                "large_number", write_header=True, x_varname='y')
        self.assertTrue(df_test._ended)
        gold_df = pandas.read_csv("{}/combine_basic_x.csv".format(
            self.__goldpath))
        self.assertTrue(df_test._final_df.equals(gold_df),
            msg="Pandas dataframe is different from gold CSV for x variable usage.")

    def testBilinear(self):
        """
        Test bilinear usage.
        """
        df_test = combine_csv.CombineCSV(self.__basename, "remove_me_54.csv",
                "large_number", write_header=True, x_varname='y',
                timefile=True, bilinear=True)
        self.assertTrue(df_test._ended)
        gold_df = pandas.read_csv("{}/combine_bilinear.csv".format(
            self.__goldpath))

        self.assertTrue(df_test._final_df.shape == gold_df.shape,
            msg="Pandas dataframe size is different from gold CSV for bilinear usage.")

        # We used to use DataFrame.equals for a comparison here, but because a
        # newer version of Pandas resulted in a very small diff in the column
        # data, we can't use it anymore. Therefore - we need to do a more fuzzy
        # check on both the columns and th data.
        data_difference = df_test._final_df.to_numpy() - gold_df.to_numpy()
        column_difference = df_test._final_df.columns.to_numpy(dtype=numpy.double) \
                            - gold_df.columns.to_numpy(dtype=numpy.double)
        self.assertTrue(abs(data_difference.max()) < 1e-10,
            msg="Pandas data is different from gold CSV for bilinear usage.")
        self.assertTrue(abs(column_difference.max()) < 1e-10,
            msg="Pandas column data is different from gold CSV for bilinear usage.")

    def testBasenameError(self):
        """
        Test exception when bad basename is provided.
        """
        with self.assertRaises(combine_csv.CombineCSV.CombineError) as cerr:
            df_test = combine_csv.CombineCSV('bad_basename_54',
                    "remove_me_54.csv", "large_number")
        self.assertEqual(cerr.exception._name, "BasenameError")

    def testStepBoundsError(self):
        """
        Test exception when mismatch of steps are provided.
        """
        with self.assertRaises(combine_csv.CombineCSV.CombineError) as cerr:
            df_test = combine_csv.CombineCSV(self.__basename,
                    "remove_me_54.csv", "large_number", lastn=2, endt=1)
        self.assertEqual(cerr.exception._name, "StepBoundsError")

    def testXVariableError(self):
        """
        Test exception when bad "x" variable name is provided.
        """
        with self.assertRaises(combine_csv.CombineCSV.CombineError) as cerr:
            df_test = combine_csv.CombineCSV(self.__basename,
                    "remove_me_54.csv", "large_number",
                    x_varname='bad_x54_name')
        self.assertEqual(cerr.exception._name, "XVariableError")

    def testInconsistentError(self):
        """
        Test exception when data rows are not consistent.
        """
        with self.assertRaises(combine_csv.CombineCSV.CombineError) as cerr:
            df_test = combine_csv.CombineCSV("{}54_bad_".format(
                self.__basename), "remove_me_54.csv", "large_number",
                    x_varname='x')
        self.assertEqual(cerr.exception._name, "InconsistentError")

    def testYVariableError(self):
        """
        Test exception when bad "y" variable name is provided.
        """
        with self.assertRaises(combine_csv.CombineCSV.CombineError) as cerr:
            df_test = combine_csv.CombineCSV(self.__basename,
                    "remove_me_54.csv", "bad_y54_name")
        self.assertEqual(cerr.exception._name, "YVariableError")

if __name__ == '__main__':
    import sys
    sys.path.append("..")
    import combine_csv
    unittest.main(module=__name__, verbosity=2)
