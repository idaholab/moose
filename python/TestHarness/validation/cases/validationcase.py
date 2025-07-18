#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: disable=invalid-name
import inspect
import traceback
import typing
import sys
import json
from numbers import Number
from typing import Any, Optional, Tuple, Union
from dataclasses import asdict

import numpy as np

from FactorySystem.MooseObject import MooseObject
from FactorySystem.InputParameters import InputParameters

from TestHarness.validation.dataclasses import *
from TestHarness.validation.exceptions import *
from TestHarness.validation.utils import ExtendedEnum

class ValidationCase(MooseObject):
    """
    Base class for a set of validation tests that can be attached
    to a TestHarness test case.
    """

    @staticmethod
    def validParams():
        return MooseObject.validParams()

    # Output format for all numbers
    number_format = '.3E'

    # Default value for absolute zero
    DEFAULT_ABS_ZERO = 1.0e-10

    class Status(str, ExtendedEnum):
        """
        The possible statuses for a validation result.
        """
        # Successful test
        OK = 'OK'
        # Failed test
        FAIL = 'FAIL'
        # Skipped test
        SKIP = 'SKIP'

    def __init__(self, params: Optional[InputParameters] = None,
                 tester_outputs: Optional[list] = None):
        MooseObject.__init__(self, params)

        # Result storage
        self._results: list[ValidationCase.Result] = []
        # Data storage
        self._data: dict[str, ValidationCase.Data] = {}
        # The current test method that is being executed
        self._current_test: str = None
        # Whether or not the current execution is not a validation case
        self._current_not_validation: Optional[bool] = None
        # The current outputs from the test run, if any
        self._tester_outputs: Optional[list] = tester_outputs

    def getOutputFiles(self) -> list[str]:
        """
        Get the output files that this validation case consumes
        """
        return []

    def addResult(self, status: Status, message: str, **kwargs) -> None:
        """
        Adds a validation result to the database.

        Args:
            status: The status (ok, fail, etc)
            message: Message associated with the status
        Keyword arguments:
            Additional arguments passed to a Result
        """
        assert isinstance(status, self.Status)
        assert isinstance(message, str)
        status_value = ValidationResult(status=status, message=message,
                                        test=self._current_test, **kwargs)

        prefix = '' if status_value.data_key is None else f'{status_value.data_key}: '
        print(f'[{status.value:>4}] {prefix}{message}')
        self._results.append(status_value)

    def _addData(self, data_type: typing.Type, key: str, value: Any,
                 description: str, **kwargs) -> ValidationData:
        """
        Internal method for creating and inserting data.

        Args:
            data_type: The underlying data type (should derive from Data)
            key: The key for the data
            value: The value for the data
            description: A description for the data
        Keyword arguments:
            Additional arguments to pass to the build
        Returns:
            The built Data object
        """
        if key in self._data:
            raise ValidationDataKeyAlreadyExists(key)

        data = data_type(value=value,
                         key=key,
                         description=description,
                         test=self._current_test,
                         **kwargs)

        # Make sure that we can dump all of the data. The TestHarness
        # relies on being able to call dump on these types, so we
        # would rather die here instead of later
        json.dumps(asdict(data))

        self._data[key] = data
        return data

    def addData(self, key: str, value: Any, description: str, **kwargs) -> None:
        """
        Adds a piece of arbitrary typed data to the validation data.

        The data type must be JSON serializable.

        Args:
            key: The key to store the data
            value: The value of the data
            description: Human readable description of the data
        Keyword arguments:
            Additional arguments passed to Data
        """
        if not isinstance(description, str):
            raise TypeError('description is not of type str')

        self._addData(ValidationData, key, value, description, **kwargs)

    @staticmethod
    def toFloat(value: Any, context: Optional[str] = None) -> float:
        """
        Converts the given value to float, if possible.

        If context is provided, will include it as the prefix
        to the thrown exception
        """
        try:
            return float(value)
        except Exception as e:
            if context:
                raise type(e)(f'{context}: {e}') from e
            raise

    @staticmethod
    def checkBounds(value: float,
                    min_bound: float,
                    max_bound: float,
                    units: Union[str, None]) -> Tuple[Status, str]:
        """
        Performs bounds checking for a scalar value and associates
        a pass/fail with a status and a message

        Args:
            value: The value to check
            min_bound: The minimum bound
            max_bound: The max bound
            units (optional): Units to add to the message
        Returns:
            Status: The associated status (ok or fail)
            str: Message associated with the check
        """
        value = ValidationCase.toFloat(value, 'value')
        min_bound = ValidationCase.toFloat(min_bound, 'min_bound')
        max_bound = ValidationCase.toFloat(max_bound, 'max_bound')
        if min_bound > max_bound:
            raise ValueError('min_bound greater than max_bound')

        success = value >= min_bound and value <= max_bound
        status = ValidationCase.Status.OK if success else ValidationCase.Status.FAIL

        units = f' {units}' if units is not None else ''
        number_format = ValidationCase.number_format
        message = [f'value {value:{number_format}}{units}']
        message += [('within' if success else 'out of') + ' bounds;']
        message += [f'min = {min_bound:{number_format}}{units},']
        message += [f'max = {max_bound:{number_format}}{units}']

        return status, ' '.join(message)

    @staticmethod
    def checkRelativeError(value: float,
                           nominal: float,
                           rel_err: float,
                           units: Union[str, None],
                           abs_zero: float = 1.e-10) -> Tuple[Status, str]:
        """
        Performs relative error checking for a scalar value and associates
        a pass/fail with a status and a message

        Args:
            value: The value to check
            nominal: The nominal value
            rel_err: The relative error
            units (optional): Units to add to the message
        Keyword args:
            abs_zero (optional): The value to use as absolute zero
        Returns:
            Status: The associated status (ok or fail)
            str: Message associated with the check
        """
        value = ValidationCase.toFloat(value, 'value')
        nominal = ValidationCase.toFloat(nominal, 'nominal')
        rel_err = ValidationCase.toFloat(rel_err, 'rel_err')
        if rel_err <= 0.0:
            raise ValueError('rel_err not positive')
        abs_zero = ValidationCase.toFloat(abs_zero, 'abs_zero')
        if abs_zero < 0.0:
            raise ValueError('abs_zero is negative')

        units = f' {units}' if units is not None else ''
        number_format = ValidationCase.number_format
        message = [f'value {value:{number_format}}{units} relative error']

        if abs(value) < abs_zero:
            value = 0.0
        if abs(nominal) < abs_zero:
            nominal = 0.0

        if value == 0.0 and nominal == 0.0:
            status = ValidationCase.Status.OK
            message += ['skipped due to absolute zero']
        else:
            error = 0.0
            max_val = max(abs(value), abs(nominal))
            if max_val > 0.0:
                error = abs((nominal - value) / max_val)
            success = error < rel_err
            status = ValidationCase.Status.OK if success else ValidationCase.Status.FAIL

            message += [f'{error:{number_format}} ' + ('<' if success else '>')]
            message += [f'required {rel_err:{number_format}}']

        return status, ' '.join(message)

    def addScalarData(self, key: str, value: Number, description: str, units: Optional[str],
                      abs_zero: float = DEFAULT_ABS_ZERO, **kwargs) -> None:
        """
        Adds a piece of scalar (float or int) data to the validation data.

        Will also perform checking on the data if bounds are set and
        store an associated Result.

        Args:
            key: The key to store the data
            value: The value of the data
            description: Human readable description of the data
            units: Human readable units for the data (can be None)
        Keyword arguments:
            abs_zero: Absolute zero to use in comparisons
            Additional arguments passed to ScalarData (bounds, nominal, rel_err, etc)
        """
        value = self.toFloat(value, 'value')
        if not isinstance(description, str):
            raise TypeError('description: not of type str')
        if units is not None and not isinstance(units, str):
            raise TypeError('units: not of type str or None')
        bounds = kwargs.get('bounds')
        if bounds is not None:
            if not isinstance(bounds, tuple):
                raise TypeError('bounds: not of type tuple')
            if not len(bounds) == 2:
                raise TypeError('bounds: not of length 2 (min and max)')
            kwargs['bounds'] = (self.toFloat(bounds[0], 'bounds min'),
                                self.toFloat(bounds[1], 'bounds max'))
        for k in ['nominal', 'rel_err']:
            v = kwargs.get(k)
            if v is not None:
                kwargs[k] = self.toFloat(v, k)
        abs_zero = self.toFloat(abs_zero, 'abs_zero')

        # Only add this information if it is used
        if kwargs.get('rel_err') is not None:
            kwargs['abs_zero'] = abs_zero

        data = self._addData(ValidationScalarData, key, value, description, units=units, **kwargs)

        result_kwargs = {'data_key': key,
                         'validation': kwargs.pop('validation', True)}

        if data.rel_err is not None:
            if data.nominal is None:
                raise KeyError("Must provide 'nominal' with 'rel_err'")
            status, message = self.checkRelativeError(data.value,
                                                      data.nominal,
                                                      data.rel_err,
                                                      data.units,
                                                      abs_zero=abs_zero)
            self.addResult(status, message, **result_kwargs)

        if data.bounds is not None:
            status, message = self.checkBounds(data.value, data.bounds[0],
                                               data.bounds[1], data.units)
            self.addResult(status, message, **result_kwargs)

    @staticmethod
    def toListFloat(value: Any, context: Optional[str] = None) -> list[float]:
        """
        Attempts to convert the given value to a one-dimensional
        list of floating point values

        Args:
            value: The number-like list value to convert
            context (optional): Optional context to prefix exceptions with
        Returns:
            list[float]: The converted value
        """
        prefix = f'{context} ' if context else ''
        try:
            arr = np.array(value, dtype=np.float64)
        except Exception as e:
            raise TypeError(f'{prefix}array conversion failed') from e
        if arr.ndim != 1:
            raise TypeError(f'{prefix}not one-dimensional')
        nan_ind = np.argwhere(np.isnan(arr)).reshape((-1))
        if nan_ind.size > 0:
            raise ValueError(f'{prefix}value(s) at indices {nan_ind} are nan')
        return arr.tolist()

    def addVectorData(self, key: str, x: ValidationVectorDataInputType,
                      value: ValidationVectorDataInputType, **kwargs) -> None:
        """
        Adds a piece of vector (float or int) data to the validation data.

        Will also perform checking on the data if bounds are set and
        store an associated Result.

        For x and value (ValidationVectorDataInputType), each should be provided as a
        three-length tuple, where the first entry is the data, the second
        is the description of the data, and the third is the units of the data
        (optional, can be None)

        Args:
            key: The key to store the data
            x: The independent data (see ValidationVectorDataInputType above)
            value: The dependent data (see ValidationVectorDataInputType above)
        Keyword arguments:
            Additional arguments passed to VectorData (bounds, nominal, etc)
        """
        for k in ['x', 'value']:
            v = locals()[k]
            if not isinstance(v, tuple):
                raise TypeError(f'{k}: not a tuple')
            if not len(v) == 3:
                raise TypeError(f'{k}: not of length 3 (value, description, units)')
            values, description, units = v
            if not isinstance(description, str):
                raise TypeError(f'{k}: second entry (description) not of type str')
            if units is not None and not isinstance(units, str):
                raise TypeError(f'{k}: third entry (units) is not of type str or None')

        # Check and convert x and data values
        values, description, units = value
        values = self.toListFloat(values, 'value: first entry (values)')
        x_values = self.toListFloat(x[0], 'x: first entry (values)')
        if len(values) != len(x_values):
            raise ValueError('Length of x and value values not the same')

        # Check bounds
        bounds = kwargs.get('bounds')
        if bounds is not None:
            if not isinstance(bounds, tuple):
                raise TypeError('bounds: not of type tuple')
            if not len(bounds) == 2:
                raise TypeError('bounds: not of length 2 (min and max)')
            kwargs['bounds'] = (self.toListFloat(bounds[0], 'bounds min:'),
                                self.toListFloat(bounds[1], 'bounds max:'))
            bounds = kwargs['bounds']
            if len(bounds[0]) != len(values):
                raise ValueError('bounds: min not same length as data')
            if len(bounds[1]) != len(values):
                raise ValueError('bounds: max not same length as data')

        # Check nominal
        nominal = kwargs.get('nominal')
        if nominal is not None:
            kwargs['nominal'] = self.toListFloat(nominal, 'nominal:')
            if len(kwargs['nominal']) != len(values):
                raise TypeError('nominal: not same length as data')

        # rel_err not supported yet
        if kwargs.get('rel_err') is not None:
            raise KeyError("'rel_err' not supported")

        # Store data
        data = self._addData(ValidationVectorData, key, values, description,
                             units=units, x=x_values, x_description=x[1], x_units=x[2],
                             **kwargs)

        result_kwargs = {'data_key': key,
                         'validation': kwargs.pop('validation', True)}

        if data.bounds is not None:
            for i, v in enumerate(values):
                status, message = self.checkBounds(v, data.bounds[0][i], data.bounds[1][i],
                                                   data.units)
                x_units = f' {data.x_units}' if data.x_units is not None else ''
                message = f'x = {x_values[i]:{self.number_format}}{x_units} (index {i}) {message}'
                self.addResult(status, message, **result_kwargs)

    @property
    def results(self) -> list[ValidationResult]:
        """
        Get all of the results
        """
        return self._results

    @property
    def data(self) -> dict[str, ValidationData]:
        """
        Get all of the data
        """
        return self._data

    def getNumResults(self) -> int:
        """
        Get the number of results populated
        """
        return len(self._results)

    def getNumResultsByStatus(self, status: Status) -> int:
        """
        Get the number of results with a specific status

        Args:
            status: The status
        """
        return len([r for r in self.results if r.status == status])

    def run(self):
        """
        Runs all of the tests. Stores results and data.
        """
        all_functions = [v[0] for v in inspect.getmembers(self.__class__,
                                                          predicate=inspect.isfunction) if len(v)]
        test_functions = [v for v in all_functions if v.startswith('test')]
        if not test_functions:
            raise ValidationNoTestsDefined(self)

        name = self.__class__.__name__
        def print_prefixed(msg: str) -> None:
            print('-' * 2 + ' ', msg)
        print_prefixed(f'Running {len(test_functions)} test case(s) in {name}')

        if 'initialize' in all_functions:
            print_prefixed(f'Running {name}.initialize')
            self.initialize() # pylint: disable=no-member

        run_exceptions = []
        for function in test_functions:
            self._current_test = f'{name}.{function}'
            self._current_not_validation = False

            print_prefixed(f'Running {self._current_test}')
            try:
                getattr(self, function)()
            except Exception as e: # pylint: disable=broad-exception-caught
                # Print to stdout so that it is mingled in
                # order with the rest of the output
                traceback.print_exc(file=sys.stdout)
                run_exceptions.append(e)
            else:
                if not any(r.test == self._current_test for r in self.results):
                    raise ValidationTestMissingResults(self, function)

            self._current_test = None
            self._current_not_validation = True

        if 'finalize' in all_functions:
            print_prefixed(f'Running {name}.finalize')
            self.finalize() # pylint: disable=no-member

        summary = f'Acquired {len(self.data)} data value(s), '
        summary += f'{self.getNumResults()} result(s): '
        results = []
        for status in ValidationCase.Status.list():
            results.append(f'{self.getNumResultsByStatus(status)} {status.name.lower()}')
        if run_exceptions:
            results.append(f'{len(run_exceptions)} exception(s)')
        summary += ', '.join(results)
        print_prefixed(summary)

        if run_exceptions:
            raise ValidationTestRunException(run_exceptions)

    def getTesterOutputs(self, extension: str = None) -> list[str]:
        """
        Gets the output files that were tested by the Tester
        executing this validation case.

        Keyword arguments:
            extension: Extension to filter the files with
        """
        assert self._tester_outputs is not None
        files = self._tester_outputs
        if extension is not None:
            assert isinstance(extension, str)
            files = [v for v in files if v.endswith(extension)]
        return files
