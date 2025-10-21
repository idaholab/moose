#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: disable=logging-fstring-interpolation

from dataclasses import dataclass
from numbers import Number
from time import sleep
from typing import Any, Iterable, Optional, Tuple, Type
import logging

import numpy as np
import numpy.typing as npt

from moosecontrol.exceptions import ControlNotWaiting, UnexpectedFlag
from moosecontrol.validation import check_response_data
from moosecontrol.runners import BaseRunner
from moosecontrol.runners.baserunner import WebServerControlResponse

# Common logger for the MooseControl
logger = logging.getLogger('MooseControl')

@dataclass
class MooseControlManager:
    """
    Context manager for MooseControlNew.
    """
    # The underlying MooseControlNew
    control: 'MooseControlNew'

class MooseControlNew:
    """
    Helper for interacting with the MOOSE WebServerControl.
    """
    def __init__(self, runner: BaseRunner):
        assert isinstance(runner, BaseRunner)

        # The underlying runner
        self._runner: BaseRunner = runner

    @property
    def runner(self) -> BaseRunner:
        """
        Get the underlying runner.
        """
        return self._runner

    @property
    def poll_time(self) -> float:
        """
        Get the time between polls in seconds.
        """
        return self.runner.poll_time

    def initialize(self):
        """
        Initializes the underlying runner.

        Must be called before interacting with the process;
        context manager calls initialize() on enter.
        """
        self.runner.initialize()

    def finalize(self):
        """
        Finalizes the underlying runner.

        Should be called when done to gracefully cleanup;
        context manager calls finalize() on exit
        """
        self.runner.finalize()

    def cleanup(self):
        """
        Performs cleanup when a non-graceful exit is needed
        such as when an exception is thrown.
        """
        self.runner.cleanup()

    def __enter__(self) -> MooseControlManager:
        self.initialize()
        return MooseControlManager(control=self)

    def __exit__(self, exc_type, exc_value, exc_traceback):
        # if exc_type is not None:
        #     logger.warning('')
        self.finalize()

    def get_waiting_flag(self) -> Optional[str]:
        """
        Get the current EXECUTE_ON flag that the WebServerControl
        is waiting on, if any.
        """
        response = self.runner.get('waiting', require_status=200)
        check_response_data(
            response,
            [('waiting', bool)],
            [('execute_on_flag', str)]
        )
        data = response.data

        # waiting = False; not waiting
        if not data['waiting']:
            assert 'execute_on_flag' not in data
            return None

        # waiting = True; get flag
        return data['execute_on_flag']

    def is_waiting(self) -> bool:
        """
        Get whether or not the control is currently waiting.
        """
        return self.get_waiting_flag() is not None

    def require_waiting(self):
        """
        Raises ControlNotWaiting if the control is not currently waiting.
        """
        if not self.is_waiting():
            raise ControlNotWaiting

    def get(self, path: str, require_status: int = 200) -> WebServerControlResponse:
        """
        Send a GET request to the server.

        Requires that the server be waiting.

        Parameters
        ----------
        path : str
            The path to GET to.

        Optional Parameters
        -------------------
        require_status : Optional[int]
            Check that the status code is this if set; default 200.

        Returns
        -------
        WebServerControlResponse:
            The combined response, along with the JSON data if any.
        """
        self.require_waiting()
        return self.runner.get(path, require_status=require_status)

    def post(self, path: str, data: dict,
             require_status: int = 200) -> WebServerControlResponse:
        """
        Send a POST request to the server.

        Requires that the server be waiting.

        Parameters
        ----------
        path : str
            The path to POST to.
        data : dict
            The JSON data to send.

        Optional Parameters
        -------------------
        require_status : int
            Check that the status code is this if set; default 200.

        Returns
        -------
        WebServerControlResponse:
            The combined response, along with the JSON data if any.
        """
        self.require_waiting()
        return self.runner.post(path, data, require_status=require_status)

    def set_continue(self):
        """
        Tells the control to continue.
        """
        logger.info('Sending continue to server')

        ws_response = self.get('continue')
        assert not ws_response.has_data()

    def set_terminate(self):
        """
        Tells the control to terminate gracefully.
        """
        logger.info('Sending terminate to server')

        ws_response = self.get('terminate')
        assert not ws_response.has_data()

    def wait(self, flag: Optional[str] = None) -> str:
        """
        Waits for the webserver and returns once the
        webserver is waiting.

        Additional Parameters
        ---------------------
        flag : Optional[str]
            The flag to wait for, if any.

        Returns
        -------
        str
            The execute on flag that the server is waiting on.
        """
        assert isinstance(flag, (str, type(None)))

        message = 'Waiting for the server'
        if flag is not None:
            message += f' to be at flag {flag}'
        logger.info(message)

        while True:
            if (current_flag := self.get_waiting_flag()) is not None:
                logger.info(f'Server is waiting at flag {current_flag}')

                if flag is not None and current_flag != flag:
                    raise UnexpectedFlag(current_flag)

                return current_flag

            sleep(self.poll_time)

    def get_postprocessor(self, name: str) -> float:
        """
        Gets a postprocessor value.
        """
        assert isinstance(name, str)
        logger.debug(f'Getting postprocessor value for "{name}"')

        response = self.post('get/postprocessor', {'name': name})
        check_response_data(response, [('value', (int, float))])
        return float(response.data['value'])

    def get_reporter_value(self, name: str) -> Any:
        """
        Gets a reporter value.
        """
        assert isinstance(name, str)
        logger.debug(f'Getting reporter value for "{name}"')

        response = self.post('get/reporter', {'name': name})
        check_response_data(response, [('value', Any)])
        return response.data['value']

    def get_time(self) -> float:
        """
        Gets the current simulation time.
        """
        logger.debug('Getting simulation time')

        response = self.get('get/time')
        check_response_data(response, [('time', (int, float))])
        return float(response.data['time'])

    def get_dt(self) -> float:
        """
        Gets the current simulation timestep.
        """
        logger.debug('Getting simulation timestep')

        response = self.get('get/dt')
        check_response_data(response, [('dt', (int, float))])
        return float(response.data['dt'])

    def _set_controllable(self, path: str, cpp_type: str,
                          python_types: Tuple[Type, ...], value: Any):
        """
        Internal method for setting a controllable value.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        cpp_type : str
            The C++ type of the value.
        python_types : Tuple[Type, ...]
            Allowed types for the input values.
        value : Any
            The value to set.
        """
        assert isinstance(path, str)
        assert isinstance(cpp_type, str)
        assert isinstance(python_types, Tuple)
        logger.debug(f'Setting controllable value "{path}"')

        data = {'name': path, 'value': value, 'type': cpp_type}
        self.post('set/controllable', data, require_status=201)

    def _set_controllable_scalar(self, path: str, cpp_type: str,
                                 python_types: Tuple[Type, ...],
                                 value: Any,
                                 python_value_type: Optional[Type] = None):
        """
        Internal method for setting a scalar controllable value.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        cpp_type : str
            The C++ type of the value.
        python_types : Tuple[Type, ...]
            Allowed types for the input values.
        value : Any
            The value to set.

        Additional Parameters
        ---------------------
        python_value_type : Optional[Type]
            The type to convert the value to, if any.
        """
        assert all(isinstance(v, Type) for v in python_types)
        assert isinstance(python_value_type, (Type, type(None)))
        if not isinstance(value, python_types):
            raise TypeError(
                f'Type {type(value).__name__}'
                ' is not of allowed type(s) ' +
                ", ".join([v.__name__ for v in python_types])
            )

        if python_value_type is not None:
            value = python_value_type(value)
        self._set_controllable(path, cpp_type, python_types, value)

    def set_controllable_bool(self, path: str, value: bool):
        """
        Sets a controllable bool parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : bool
            The value to set.
        """
        self._set_controllable_scalar(path, 'bool', (bool,), value)

    def set_controllable_int(self, path: str, value: int):
        """
        Sets a controllable int parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : int
            The value to set.
        """
        self._set_controllable_scalar(path, 'int', (int,), value)

    def set_controllable_real(self, path: str, value: Number):
        """
        Sets a controllable Real parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : Number
            The value to set.
        """
        self._set_controllable_scalar(path, 'Real', (Number,), value, float)

    def set_controllable_string(self, path: str, value: str):
        """
        Sets a controllable std::string parameter.

        Must be waiting.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : str
            The value to set.
        """
        self._set_controllable_scalar(path, 'std::string', (str,), value)

    def _set_controllable_vector(self, path: str, cpp_type: str,
                                 python_types: Tuple[Type, ...],
                                 value: Iterable[Any],
                                 python_value_type: Optional[Type] = None):
        """
        Internal method for setting a std::vector controllable value.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        cpp_type : str
            The C++ type of the value in the vector container.
        python_types : Tuple[Type, ...]
            Allowed types for the input values.
        value : Iterable[Any]
            The value to set.

        Additional Parameters
        ---------------------
        python_value_type : Optional[Type]
            The type to convert the value to, if any.
        """
        assert all(isinstance(v, Type) for v in python_types)
        assert isinstance(python_value_type, (Type, type(None)))
        for i, v in enumerate(value):
            if not isinstance(v, python_types):
                raise TypeError(
                    f'At index {i}: type {type(value).__name__}'
                    ' is not of allowed type(s) ' +
                    ", ".join([v.__name__ for v in python_types])
                )

        value = [v for v in value]
        if python_value_type is not None:
            value = [python_value_type(v) for v in value]

        cpp_type = f'std::vector<{cpp_type}>'
        self._set_controllable(path, cpp_type, python_types, value)

    def set_controllable_vector_int(self, path: str, value: Iterable[int]):
        """
        Sets a controllable std::vector<int> parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : Iterable[int]
            The value to set.
        """
        self._set_controllable_vector(path, 'int', (int,), value)

    def set_controllable_vector_real(self, path: str, value: Iterable[Number]):
        """
        Sets a controllable std::vector<Real> parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : Iterable[Number]
            The value to set.
        """
        self._set_controllable_vector(path, 'Real', (Number,), value, float)

    def set_controllable_vector_string(self, path: str, value: Iterable[str]):
        """
        Sets a controllable std::vector<std::string> parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : Iterable[str]
            The value to set.
        """
        self._set_controllable_vector(path, 'std::string', (str,), value)

    def set_controllable_matrix(self, path: str, value: npt.ArrayLike):
        """
        Sets a controllable RealEigenMatrix parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : npt.ArrayLike
            The value to set.
        """
        try:
            array = np.array(value, dtype=np.float64)
            if len(array.shape) == 1:
                array = array.reshape((1, -1))
            assert len(array.shape) == 2
        except Exception as e:
            raise ValueError('Value not convertible to a 1- or 2-D array') from e
        self._set_controllable_scalar(
            path,
            'RealEigenMatrix',
            (list,),
            array.tolist()
        )
