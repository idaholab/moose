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
import math
from numbers import Number
from numpy.typing import NDArray
import numpy as np
from typing import Optional, Tuple, Union
from enum import Enum
from dataclasses import dataclass
from FactorySystem.MooseObject import MooseObject
from FactorySystem.InputParameters import InputParameters

class ExtendedEnum(Enum):
    """
    Extension to Enum that allows getting
    all possible enumerations in a list
    """
    @classmethod
    def list(cls) -> list:
        """
        Get all possible enumerations in a list
        """
        return list(cls)

# The valid numeric data types
NumericDataType = Union[float, list[float]]
# Valid numeric vector types
NumericVectorType = Union[NDArray[np.float64], list[float]]
# Input type for addVectorData
VectorDataInputType = Tuple[NumericVectorType, str, Optional[str]]

class DataKeyAlreadyExists(Exception):
    """
    Exception for when a data key already exists
    """
    def __init__(self, key: str):
        self.key: str = key
        super().__init__(f'Data "{self.key}" is already registered')

class NoTestsDefined(Exception):
    """
    Exception for when no tests were defined
    """
    def __init__(self, obj):
        super().__init__(f'No test functions defined in {obj.__class__.__name__}')

class TestMissingResults(Exception):
    """
    Exception for when a test was ran without any results
    """
    def __init__(self, obj, function):
        super().__init__(f'No results reported in {obj.__class__.__name__}.{function}')

class TestRunException(Exception):
    """
    Exception for when an exception was found when running a test
    """
    pass

class ValidationCase(MooseObject):
    @staticmethod
    def validParams():
        return MooseObject.validParams()

    # Output format for all numbers
    number_format = '.3E'

    """
    Base class for a set of validation tests that can be attached
    to a TestHarness test case.
    """
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

    @dataclass(kw_only=True)
    class Result:
        """
        Data structure that stores the information
        about a single validation result.
        """
        # The status
        status: None
        # A human readable message
        message: str
        # The test method the result came from
        test: str
        # Whether or not this result is considered
        # a validation result (enables running verification
        # cases but not storing them in a database)
        validation: bool = True
        # The data that this result is attached to,
        # if any
        data_key: Optional[str] = None

    @dataclass(kw_only=True)
    class Data:
        """
        Base data structure that stores the information
        about a piece of validation data to be stored
        """
        def __post_init__(self):
            assert isinstance(self.key, str)
            assert isinstance(self.description, str)
            assert isinstance(self.test, (str, type(None)))
            assert isinstance(self.validation, bool)

            try:
                json.dumps(self.value)
            except (TypeError, OverflowError):
                raise TypeError(f'Data type "{type(self.value)}" is not JSON serializable')

        # The data key
        key: str
        # The data
        value: typing.Any
        # Human readable description of the data
        description: str
        # The test that added this data, if any
        test: Optional[str]
        # Whether or not this result is considered
        # a validation result (enables running verification
        # cases but not storing them in a database)
        validation: bool = True

    @dataclass(kw_only=True)
    class NumericData(Data):
        """
        Data structure that stores the information about
        a piece of numeric validation data that can be checked
        """
        def __post_init__(self):
            super().__post_init__()
            assert isinstance(self.units, (str, type(None)))
            if self.nominal is not None:
                assert type(self.value) == type(self.nominal)
                if not isinstance(self.value, float):
                    assert len(self.value) == len(self.nominal)
            if self.bounds is not None:
                assert isinstance(self.bounds, tuple)
                assert len(self.bounds) == 2
                if not isinstance(self.value, float):
                    assert len(self.value) == len(self.bounds[0])
                    assert len(self.value) == len(self.bounds[1])

        # Units for the data, if any
        units: Optional[str]
        # A nominal value for this data; unused
        # in the test but useful in postprocessing
        nominal: Optional[NumericDataType] = None
        # Bounds for the data (min and max)
        bounds: Optional[Tuple[NumericDataType, NumericDataType]] = None

    @dataclass(kw_only=True)
    class VectorNumericData(NumericData):
        """
        Data structure that stores the information about
        a piece of array numeric validation data that can be checked
        """
        def __post_init__(self):
            super().__post_init__()
            assert isinstance(self.x, list)
            assert type(self.x) == type(self.value)
            assert len(self.x) == len(self.value)
            assert isinstance(self.description, str)
            assert isinstance(self.x_units, (str, type(None)))

        # The x values for the data
        x: list[float]
        # The description for the x data
        x_description: str
        # Units for the x data, if any
        x_units: Optional[str]

    # Stores classes that are instantiated that
    # are derived from this class
    _subclasses = []

    @classmethod
    def getSubclasses(cls) -> list:
        """
        Get the classes that are loaded that are derived
        from this class.

        Used within a TestHarness Job to load the user's
        derived test cases.
        """
        return list(cls._subclasses)

    def __init_subclass__(cls):
        """
        Helper for storing derived classes so that they
        can be loaded with getSubclasses
        """
        ValidationCase._subclasses.append(cls)

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
        status_value = self.Result(status=status, message=message,
                                   test=self._current_test, **kwargs)

        prefix = '' if status_value.data_key is None else f'{status_value.data_key}: '
        print(f'[{status.value:>4}] {prefix}{message}')
        self._results.append(status_value)

    def _addData(self, data_type: typing.Type, key: str, value: typing.Any,
                 description: str, **kwargs) -> Data:
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
            raise DataKeyAlreadyExists(key)

        data = data_type(value=value,
                         key=key,
                         description=description,
                         test=self._current_test,
                         **kwargs)
        self._data[key] = data
        return data

    def addData(self, key: str, value: typing.Any, description: str, **kwargs) -> None:
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

        self._addData(self.Data, key, value, description)

    @staticmethod
    def toFloat(value: typing.Any, context: Optional[str] = None) -> float:
        """
        Converts the given value to float, if possible.

        If context is provided, will include it as the prefix
        to the thrown exception
        """
        if not isinstance(value, float):
            try:
                value = float(value)
            except Exception as e:
                if context:
                    raise type(e)(f'{context}: {e}') from e
                raise
        return value

    @staticmethod
    def checkBounds(value: float,
                    min_bound: float,
                    max_bound: float,
                    units: Optional[str]) -> Tuple[Status, str]:
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

    def addScalarData(self, key: str, value: Number, description: str,
                      units: Optional[str], **kwargs) -> None:
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
            Additional arguments passed to NumericData (bounds, nominal, etc)
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
        nominal = kwargs.get('nominal')
        if nominal is not None:
            kwargs['nominal'] = self.toFloat(nominal, 'nominal')

        data = self._addData(self.NumericData, key, value, description, units=units, **kwargs)

        result_kwargs = {'data_key': key,
                         'validation': kwargs.pop('validation', True)}

        if data.bounds is not None:
            status, message = self.checkBounds(data.value, data.bounds[0], data.bounds[1], data.units)
            self.addResult(status, message, **result_kwargs)

    @staticmethod
    def toListFloat(value: typing.Any, context: Optional[str] = None) -> list[float]:
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
        value = arr.tolist()
        for i, v in enumerate(value):
            if math.isnan(v):
                raise ValueError(f'{prefix}value at index {i} is nan')
        return value

    def addVectorData(self, key: str, x: VectorDataInputType, value: VectorDataInputType, **kwargs) -> None:
        """
        Adds a piece of vector (float or int) data to the validation data.

        Will also perform checking on the data if bounds are set and
        store an associated Result.

        For x and value (VectorDataInputType), each should be provided as a
        three-length tuple, where the first entry is the data, the second
        is the description of the data, and the third is the units of the data
        (optional, can be None)

        Args:
            key: The key to store the data
            x: The independent data (see VectorDataInputType above)
            value: The dependent data (see VectorDataInputType above)
        Keyword arguments:
            Additional arguments passed to VectorNumericData (bounds, nominal, etc)
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
            raise ValueError(f'Length of x and value values not the same')

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

        # Store data
        data = self._addData(self.VectorNumericData, key, values, description,
                             units=units, x=x_values, x_description=x[1], x_units=x[2],
                             **kwargs)

        result_kwargs = {'data_key': key,
                         'validation': kwargs.pop('validation', True)}

        if data.bounds is not None:
            for i in range(len(values)):
                status, message = self.checkBounds(values[i], data.bounds[0][i], data.bounds[1][i],
                                                   data.units)
                x_units = f' {data.x_units}' if data.x_units is not None else ''
                message = f'x = {x_values[i]:{self.number_format}}{x_units} (index {i}) {message}'
                self.addResult(status, message, **result_kwargs)

    @property
    def results(self) -> list[Result]:
        """
        Get all of the results
        """
        return self._results

    @property
    def data(self) -> dict[str, Data]:
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
            raise NoTestsDefined(self)

        name = self.__class__.__name__
        print_prefixed = lambda msg: print('-' * 2 + ' ', msg)
        print_prefixed(f'Running {len(test_functions)} test case(s) in {name}')

        if 'initialize' in all_functions:
            print_prefixed(f'Running {name}.initialize')
            self.initialize()

        run_exceptions = 0
        for function in test_functions:
            self._current_test = f'{name}.{function}'
            self._current_not_validation = False

            print_prefixed(f'Running {self._current_test}')
            try:
                getattr(self, function)()
            except:
                # Print to stdout so that it is mingled in
                # order with the rest of the output
                traceback.print_exc(file=sys.stdout)
                run_exceptions += 1
            else:
                if not any(r.test == self._current_test for r in self.results):
                    raise TestMissingResults(self, function)

            self._current_test = None
            self._current_not_validation = True

        if 'finalize' in all_functions:
            print_prefixed(f'Running {name}.finalize')
            self.finalize()

        summary = f'Acquired {len(self.data)} data value(s), '
        summary += f'{self.getNumResults()} result(s): '
        results = []
        for status in ValidationCase.Status.list():
            results.append(f'{self.getNumResultsByStatus(status)} {status.name.lower()}')
        if run_exceptions:
            results.append(f'{run_exceptions} exception(s)')
        summary += ', '.join(results)
        print_prefixed(summary)

        if run_exceptions:
            raise TestRunException()

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
