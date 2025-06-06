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
from typing import Optional
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

data_types = typing.Union[float]

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

    @dataclass
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

    @dataclass
    class Data:
        """
        Base data structure that stores the information
        about a piece of validation data to be stored.
        """
        # The data value
        value: data_types
        # The data key
        key: str
        # Human readable description of the data
        description: str
        # The test that added this data
        test: Optional[str]
        # Units for the data, if any
        units: Optional[str]
        # Whether or not this result is considered
        # a validation result (enables running verification
        # cases but not storing them in a database)
        validation: bool = True
        # A nominal value for this data; unused
        # in the test but useful in postprocessing
        nominal: Optional[data_types] = None
        # Bounds for the data (min and max)
        bounds: Optional[tuple[data_types, data_types]] = None

        def printableValue(self) -> str:
            """
            Gets a printable version of the underlying value
            """
            raise NotImplementedError

    @dataclass
    class FloatData(Data):
        """
        Data structure for a piece of validation data
        that is of type float.
        """
        def printableValue(self) -> str:
            units = f' {self.units}' if self.units is not None else ''
            return f'{self.value:.5E}{units}'

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

    def addFloatData(self, key: str, value: float, units: Optional[str],
                     description: str, **kwargs):
        """
        Adds a piece of float data to the validation data.

        Will also perform checking on the data if bounds are set and
        store an associated Result.

        Args:
            key: The key to store the data
            value: The value of the data
            units: Human readable units for the data (can be None)
            description: Human readable description of the data
        Keyword arguments:
            Additional arguments passed to FloatData
        """
        if not isinstance(value, float):
            raise ValueError('value is not of type float')

        if key in self._data:
            raise DataKeyAlreadyExists(key)

        data = self.FloatData(value=value,
                              units=units,
                              key=key,
                              description=description,
                              test=self._current_test,
                              **kwargs)
        self._data[key] = data

        result_kwargs = {'data_key': key,
                         'validation': kwargs.pop('validation', True)}
        units = f' {data.units}' if data.units is not None else ''

        if data.bounds is not None:
            assert len(data.bounds) == 2
            min, max = data.bounds
            assert max > min
            success = value >= min and value <= max
            status = self.Status.OK if success else self.Status.FAIL
            message = [('within' if success else 'out of') + ' bounds;']
            message += [f'min = {min:.5E}{units},']
            message += [f'max = {max:.5E}{units}']
            self.addResult(status, ' '.join(message), **result_kwargs)

    @property
    def results(self) -> list['Result']:
        """
        Get all of the results
        """
        return self._results

    @property
    def data(self) -> dict[str, 'Data']:
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
                traceback.print_exc()
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
