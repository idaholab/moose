#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from typing import Optional

from pandas import DataFrame
import pandas as pd

from TestHarness.validation import ValidationCase

NoneType = type(None)

class CSVValidationCase(ValidationCase):
    """
    Derived validation case that provides an interface
    for interacting with CSV files that have golds,
    similar to what is done in CSVDiff
    """
    @staticmethod
    def validParams():
        params = ValidationCase.validParams()
        params.addRequiredParam('validation_csv', 'The CSV file')
        params.addParam('validation_rel_err', 1.0e-6, 'The relative error')
        params.addParam('validation_abs_zero', ValidationCase.DEFAULT_ABS_ZERO, 'Absolute zero cutoff used in comparisons')
        return params

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        csv = self.getParam('validation_csv')
        assert isinstance(csv, str)

        # The absolute path to the csv file
        self._csv: str = os.path.abspath(csv)
        # The absolute path to the gold csv file
        self._gold_csv: str = os.path.join(os.getcwd(), 'gold', csv)
        # The relative error to check against
        self._rel_err: float = float(self.getParam('validation_rel_err'))
        # Cutoff used in comparisons:
        self._abs_zero: float = float(self.getParam('validation_abs_zero'))
        # Whether or not initialize was called
        self._called_csv_initialize = False

        # validation_csv path must be in this directory
        if os.path.dirname(self._csv) != os.getcwd():
            raise ValueError(f'validation_csv={csv} is not in this directory')

        # Check for existance of gold
        if not os.path.exists(self._gold_csv):
            raise FileNotFoundError(f'Gold CSV file {self._gold_csv} not found')

        # Load the gold file
        self._gold_df: DataFrame = pd.read_csv(self._gold_csv)

    def initialize(self):
        if not os.path.exists(self._csv):
            raise FileNotFoundError(f'CSV file {self._csv} not found')
        self._df: DataFrame = pd.read_csv(self._csv)
        self._called_csv_initialize = True

    def _getScalarCSV(self, key: str, index: int, gold: bool) -> float:
        """
        Internal helper for getting a scalar value from the CSV file

        Args:
            key: The key in the CSV (column)
            index: The index in the CSV file to read
            gold: Whether or not to read the gold file
        """
        assert isinstance(key, str)
        assert isinstance(index, int)
        assert isinstance(gold, bool)

        # If not initialized, we don't have data
        if not self._called_csv_initialize:
            raise Exception(f'CSVValidationCase.initialize() was not called in {self.__class__.__name__}')

        df = self._gold_df if gold else self._df
        path = self._gold_csv if gold else self._csv

        try:
            value = df[key].iloc[index]
        except KeyError as e:
            raise KeyError(f'Column {key} does not exist in {path}') from e
        except IndexError as e:
            raise IndexError(f'Index {index} out of range in {path}') from e

        return float(value)

    def addScalarCSVData(self, key: str, index: int, description: str,
                         units: Optional[str], check: bool = True,
                         store_key: Optional[str] = None,
                         **kwargs) -> None:
        """
        Adds a scalar data entry from the CSV file with bounds checking
        against the gold value

        Args:
            key: The key in the CSV (column)
            index: The index in the CSV file to read (-1 for last)
            description: Human readable description of the data
            units: Human readable units for the data (can be None)
        Keyword arguments:
            check: Whether or not to check the data (default: True)
            store_key: The key to store the data as in the database,
                otherwise just use 'key'
            Remaining arguments passed to addScalarData()
        """
        assert isinstance(key, str)
        assert isinstance(index, int)
        assert isinstance(store_key, (str, NoneType))

        # Key that we're storing the data as isn't the
        # same as the column name in the CSV
        if not store_key:
            store_key = key

        # Load both values from CSV
        value = self._getScalarCSV(key, index, False)
        gold_value = self._getScalarCSV(key, index, True)

        # "nominal" value is the gold value
        kwargs['nominal'] = gold_value
        # Use abs_zero from parameters
        kwargs['abs_zero'] = self._abs_zero

        # If check=True (default), set the relative error
        # that the data will checked with
        if check:
            kwargs['rel_err'] = self._rel_err

        self.addScalarData(store_key, value, description, units, **kwargs)
