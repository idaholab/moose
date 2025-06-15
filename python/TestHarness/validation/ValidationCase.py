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
NumericDataType = Union[float, int]

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

        def printableValue(self) -> str:
            raise NotImplementedError

    @dataclass(kw_only=True)
    class NumericData(Data):
        """
        Data structure that stores the information about
        a piece of numeric validation data that can be checked
        """
        def __post_init__(self):
            super().__post_init__()
            assert isinstance(self.units, (str, type(None)))
            assert isinstance(self.nominal, (float, type(None)))
            if self.bounds is not None:
                assert isinstance(self.bounds, tuple)
                assert len(self.bounds) == 2

        # Units for the data, if any
        units: Optional[str]
        # A nominal value for this data; unused
        # in the test but useful in postprocessing
        nominal: Optional[float] = None
        # Bounds for the data (min and max)
        bounds: Optional[tuple[float, float]] = None

        def printableValue(self) -> str:
            if isinstance(self.value, float):
                units = f' {self.units}' if self.units is not None else ''
                return f'{self.value:.5E}{units}'
            raise NotImplementedError

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

        prefix = ''
        if status_value.data_key is not None:
            data = self._data[status_value.data_key]
            prefix = f'{data.key} = {data.printableValue()} '
        print(f'[{status.value:>4}] {prefix}{message}')
        self._results.append(status_value)

    def _addData(self, data_type: typing.Type, key: str, value: typing.Any,
                 description: str, **kwargs):
        """
        Internal method for creating and inserting data.
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
    def checkBounds(value: float, bounds: typing.Tuple[float, float], units: Optional[str]):
        assert isinstance(bounds, tuple)
        assert len(bounds) == 2
        min, max = bounds
        assert max > min

        success = value >= min and value <= max
        status = ValidationCase.Status.OK if success else ValidationCase.Status.FAIL

        units = f' {units}' if units is not None else ''
        message = [('within' if success else 'out of') + ' bounds;']
        message += [f'min = {min:.5E}{units},']
        message += [f'max = {max:.5E}{units}']

        return status, ' '.join(message)

    def addScalarData(self, key: str, value: NumericDataType, description: str,
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
            Additional arguments passed to NumericData
        """
        if isinstance(value, int):
            value = float(value)
        if not isinstance(value, float):
            raise TypeError('value: not of type float or int')
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
            if not isinstance(bounds[0], (int, float)):
                raise TypeError('bounds: min bounds not of type int or float')
            if not isinstance(bounds[1], (int, float)):
                raise TypeError('bounds: max bounds not of type int or float')
            kwargs['bounds'] = (float(bounds[0]), float(bounds[1]))

        data = self._addData(self.NumericData, key, value, description, units=units, **kwargs)

        result_kwargs = {'data_key': key,
                         'validation': kwargs.pop('validation', True)}

        if data.bounds is not None:
            status, message = self.checkBounds(data.value, data.bounds, data.units)
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
