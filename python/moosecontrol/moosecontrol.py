# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements the MooseControl."""

import logging
from dataclasses import dataclass
from getpass import getuser
from numbers import Number
from socket import getfqdn
from time import sleep
from typing import Any, Iterable, Optional, Tuple, Type

import numpy as np
import numpy.typing as npt

from moosecontrol.exceptions import ControlNotWaiting, UnexpectedFlag
from moosecontrol.runners import BaseRunner
from moosecontrol.runners.baserunner import WebServerControlResponse
from moosecontrol.validation import WebServerInitializedData, check_response_data

# Common logger for the MooseControl
logger = logging.getLogger("MooseControl")
# The stream handler for logging
STREAM_HANDLER = logging.StreamHandler()
# Default format to use for logs
DEFAULT_LOG_FORMAT = "%(message)s"
# Format to use for debug logs (when verbose=True)
DEBUG_LOG_FORMAT = "%(asctime)s:%(levelname)s:%(name)s: %(message)s"


@dataclass
class MooseControlContextManager:
    """Context manager for MooseControl."""

    # The underlying MooseControl
    control: "MooseControl"


class MooseControl:
    """Helper for interacting with the MOOSE WebServerControl."""

    def __init__(self, runner: BaseRunner, quiet: bool = False, verbose: bool = False):
        """
        Initialize state and setup logger if applicable.

        Arguments:
        ---------
        runner : BaseRunner
            The runner used to connect to the server.

        Additional Arguments
        --------------------
        quiet : bool
            Disables on-screen logging output if True.
        verbose : bool
            Enables on-screen debugging output if True.

        """
        assert isinstance(runner, BaseRunner)
        assert isinstance(quiet, bool)
        assert isinstance(verbose, bool)
        if quiet and verbose:
            raise ValueError("Cannot set quiet=True and verbose=True")

        # The underlying runner
        self._runner: BaseRunner = runner

        # The data received on initialize from the server
        self._initialized_data: Optional[WebServerInitializedData] = None

        # Setup logger
        if not quiet:
            logging.basicConfig(
                level=logging.DEBUG if verbose else logging.INFO,
                handlers=[STREAM_HANDLER],
                format=DEBUG_LOG_FORMAT if verbose else DEFAULT_LOG_FORMAT,
                datefmt="%H:%M:%S",
            )
            if verbose:
                logging.getLogger().setLevel(logging.DEBUG)
                logging.getLogger("urllib3.connectionpool").setLevel(logging.INFO)

    @property
    def runner(self) -> BaseRunner:
        """Get the underlying runner."""
        return self._runner

    @property
    def poll_time(self) -> float:
        """Get the time between polls in seconds."""
        return self.runner.poll_time

    @property
    def initialized(self) -> bool:
        """Whether or not we have initialized."""
        return self.runner.initialized

    @property
    def initialized_data(self) -> WebServerInitializedData:
        """Data received on /initialize with the server."""
        assert self._initialized_data is not None
        return self._initialized_data

    @property
    def control_type(self) -> str:
        """Type of WebServerControl being controlled."""
        return self.initialized_data.control_type

    @property
    def control_name(self) -> str:
        """Name of the WebServerControl being controlled."""
        return self.initialized_data.control_name

    @property
    def execute_on_flags(self) -> list[str]:
        """Execute on flags the WebServerControl is listening on."""
        return self.initialized_data.execute_on_flags

    @staticmethod
    def required_initialize_data(object: object) -> dict:
        """Get the base required data for /initialize as needed by the control."""
        return {
            "name": f"Python {object.__class__.__name__}",
            "host": getfqdn(),
            "user": getuser(),
        }

    def get_initialize_data(self) -> dict:
        """
        Get the data to add when initializing on POST /initialize.

        Takes the base data and adds on any additional data
        that a derived class may also want to initialize with.
        """
        # Base data to be included in all requests
        data = self.required_initialize_data(self)

        # Any data from a derived class, which cannot overwrite
        # the base data
        derived_data = self.additional_initialize_data()
        for key in derived_data:
            if key in data:
                raise KeyError(
                    f'Cannot override entry "{key}" in additional_initialize_data()'
                )
        data.update(derived_data)

        return data

    def additional_initialize_data(self) -> dict:
        """Entrypoint for derived classes to add additional data on initialize."""
        return {}

    def initialize(self):
        """
        Initialize the runner and waits for the process to start.

        Must be called before interacting with the process;
        context manager calls initialize() on enter.
        """
        assert not self.initialized
        initialize_data = self.get_initialize_data()
        self.runner.initialize(initialize_data)
        assert self.initialized

        # Load the data from initialize
        self._initialized_data = WebServerInitializedData(self.runner.initialized_data)

        # Output information about the control
        logger.info(
            f'Initialized connection with {self.control_type} "{self.control_name}"'
        )
        combined_flags = ", ".join(self.execute_on_flags)
        logger.info(f"Control is listening on flags: {combined_flags}")

        self.wait()

    def finalize(self):
        """
        Finalize the underlying runner.

        Should be called when done to gracefully cleanup;
        context manager calls finalize() on exit
        """
        self.runner.finalize()

    def cleanup(self):
        """Perform cleanup for a non-graceful exit."""
        self.runner.cleanup()

    def __enter__(self) -> MooseControlContextManager:
        """
        Enter a context manager for the MooseControl.

        Will initialize on enter and finalize on exit
        if an exception is not raised, otherwise will
        cleanup on exit.
        """
        self.initialize()
        return MooseControlContextManager(control=self)

    def __exit__(self, exc_type, exc_value, exc_traceback):
        """
        Exit for the context manager.

        Will finalize if an exception is not raised,
        otherwise will cleanup.
        """
        if exc_type is not None:
            logger.warning(f"Encountered {exc_type.__name__} on exit; running cleanup")
            self.cleanup()
        else:
            self.finalize()

    def get_waiting_flag(self) -> Optional[str]:
        """Get the current EXECUTE_ON flag from MOOSE, if any."""
        response = self.runner.get("waiting", require_status=200)
        check_response_data(response, [("waiting", bool)], [("execute_on_flag", str)])
        data = response.data

        # waiting = False; not waiting
        if not data["waiting"]:
            assert "execute_on_flag" not in data
            return None

        # waiting = True; get flag
        return data["execute_on_flag"]

    def is_waiting(self) -> bool:
        """Get whether or not the control is currently waiting."""
        return self.get_waiting_flag() is not None

    def require_waiting(self):
        """Raise ControlNotWaiting if the control is not waiting."""
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
        assert self.initialized
        self.require_waiting()
        return self.runner.get(path, require_status=require_status)

    def post(
        self, path: str, data: dict, require_status: int = 200
    ) -> WebServerControlResponse:
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
        assert self.initialized
        self.require_waiting()
        return self.runner.post(path, data, require_status=require_status)

    def set_continue(self):
        """Tell the control to continue."""
        logger.info("Sending continue to server")

        ws_response = self.get("continue")
        assert not ws_response.has_data()

    def set_terminate(self):
        """Tell the control to terminate gracefully."""
        logger.info("Sending terminate to server")

        ws_response = self.get("terminate")
        assert not ws_response.has_data()

    def wait(self, flag: Optional[str] = None) -> str:
        """
        Wait for the webserver to be waiting.

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

        message = "Waiting for the server"
        if flag is not None:
            message += f" to be at flag {flag}"
        logger.info(message)

        while True:
            if (current_flag := self.get_waiting_flag()) is not None:
                logger.info(f"Server is waiting at flag {current_flag}")

                if flag is not None and current_flag != flag:
                    raise UnexpectedFlag(current_flag)

                return current_flag

            sleep(self.poll_time)

    def get_postprocessor(self, name: str) -> float:
        """Get a postprocessor value by name."""
        assert isinstance(name, str)
        logger.debug(f'Getting postprocessor value for "{name}"')

        response = self.post("get/postprocessor", {"name": name})
        check_response_data(response, [("value", (int, float))])
        return float(response.data["value"])

    def get_reporter(self, name: str) -> Any:
        """Get a reporter value by name."""
        assert isinstance(name, str)
        logger.debug(f'Getting reporter value for "{name}"')

        response = self.post("get/reporter", {"name": name})
        check_response_data(response, [("value", Any)])
        return response.data["value"]

    def get_time(self) -> float:
        """Get the current simulation time."""
        logger.debug("Getting simulation time")

        response = self.get("get/time")
        check_response_data(response, [("time", (int, float))])
        return float(response.data["time"])

    def get_dt(self) -> float:
        """Get the current simulation timestep."""
        logger.debug("Getting simulation timestep")

        response = self.get("get/dt")
        check_response_data(response, [("dt", (int, float))])
        return float(response.data["dt"])

    def set_controllable(
        self, path: str, cpp_type: str, python_types: Tuple[Type, ...], value: Any
    ):
        """
        Set a generic controllable value.

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

        data = {"name": path, "value": value, "type": cpp_type}
        self.post("set/controllable", data, require_status=201)

    def set_controllable_scalar(
        self,
        path: str,
        cpp_type: str,
        python_types: Tuple[Type, ...],
        value: Any,
        python_value_type: Optional[Type] = None,
    ):
        """
        Set a generic scalar controllable value.

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
                f"Type {type(value).__name__}"
                " is not of allowed type(s) "
                + ", ".join([v.__name__ for v in python_types])
            )

        if python_value_type is not None:
            value = python_value_type(value)
        self.set_controllable(path, cpp_type, python_types, value)

    def set_bool(self, path: str, value: bool):
        """
        Set a controllable bool parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : bool
            The value to set.

        """
        self.set_controllable_scalar(path, "bool", (bool,), value)

    def set_int(self, path: str, value: int):
        """
        Set a controllable int parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : int
            The value to set.

        """
        self.set_controllable_scalar(path, "int", (int,), value)

    def set_real(self, path: str, value: float):
        """
        Set a controllable Real parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : float
            The value to set.

        """
        self.set_controllable_scalar(path, "Real", (Number,), value, float)

    def set_string(self, path: str, value: str):
        """
        Set a controllable std::string parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : str
            The value to set.

        """
        self.set_controllable_scalar(path, "std::string", (str,), value)

    def set_controllable_vector(
        self,
        path: str,
        cpp_type: str,
        python_types: Tuple[Type, ...],
        value: Iterable[Any],
        python_value_type: Optional[Type] = None,
    ):
        """
        Set a generic vector controllable value.

        Internal method to be used by the explicit vector setters.

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
                    f"At index {i}: type {type(value).__name__}"
                    " is not of allowed type(s) "
                    + ", ".join([v.__name__ for v in python_types])
                )

        value = list(value)
        if python_value_type is not None:
            value = [python_value_type(v) for v in value]

        cpp_type = f"std::vector<{cpp_type}>"
        self.set_controllable(path, cpp_type, python_types, value)

    def set_vector_int(self, path: str, value: Iterable[int]):
        """
        Set a controllable std::vector<int> parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : Iterable[int]
            The value to set.

        """
        self.set_controllable_vector(path, "int", (int,), value)

    def set_vector_real(self, path: str, value: Iterable[Number]):
        """
        Set a controllable std::vector<Real> parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : Iterable[Number]
            The value to set.

        """
        self.set_controllable_vector(path, "Real", (Number,), value, float)

    def set_vector_string(self, path: str, value: Iterable[str]):
        """
        Set a controllable std::vector<std::string> parameter.

        Parameters
        ----------
        path : str
            The path to the controllable value.
        value : Iterable[str]
            The value to set.

        """
        self.set_controllable_vector(path, "std::string", (str,), value)

    def set_realeigenmatrix(self, path: str, value: npt.ArrayLike):
        """
        Set a controllable RealEigenMatrix parameter.

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
            raise ValueError("Value not convertible to a 1- or 2-D array") from e
        self.set_controllable_scalar(path, "RealEigenMatrix", (list,), array.tolist())
