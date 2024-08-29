#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import requests
import string
import random
import subprocess
import time
import logging
import tempfile
from threading import Thread

# Common logger for the MooseControl
logger = logging.getLogger('MooseControl')

class MooseControl:
    """Helper for interacting with the MOOSE WebServerControl.

    Use this class as follows:
        control = MooseControl(...)
        control.initialize()
        <interact with the process>
        control.finalize()

    This object is tested primarily by
    test/tests/controls/web_server_control in the framework."""

    def __init__(self,
                 moose_command: list[str] = None,
                 moose_port: int = None,
                 moose_control_name: str = None,
                 inherit_environment: bool = True):
        """Constructor

        If "moose_port" is specified without "moose_command": Connect to the webserver at
        this port and do not spawn a moose process.

        If "moose_command" is specified at all, "moose_control_name" is needed in order
        to specify a command line argument to set either the port or the file socket.

        If "moose_command" is specified with "moose_port": Spawn a moose process and then
        connect to it at the specified port.

        If "moose_command" is specified without "moose_port": Spawn a moose process and then
        determine a file socket to connect to within the current working directory. This is
        the preferred method of operation.

        Parameters:
            moose_command (list[str]): The command to use to start the moose process
            moose_port (int): The webserver port to connect to
            moose_control_name (str): The name of the input control object
            inherit_environment (bool): Whether or not the MOOSE command will inherit the current shell environment
        """
        # Setup a basic logger
        logging.basicConfig(level=logging.INFO,
                            handlers=[logging.StreamHandler()],
                            format='%(name)s: %(message)s')

        # Sanity checks on input
        has_moose_command = moose_command is not None
        has_moose_port = moose_port is not None
        has_moose_control_name = moose_control_name is not None
        if not has_moose_command and not has_moose_port:
            raise ValueError('One of "moose_command" or "moose_port" must at least be provided')
        if has_moose_command and not has_moose_control_name:
            raise ValueError('"moose_control_name" must be specified with "moose_command"')
        if has_moose_command and not isinstance(moose_command, list):
            raise ValueError('"moose_command" is not a list')

        # Store inputs
        self._moose_command = moose_command
        self._moose_port = moose_port
        self._moose_control_name = moose_control_name
        self._inherit_environment = inherit_environment

        # Set defaults
        self._url = None
        self._moose_process = None
        self._moose_reader = None

        # How often we want to poll MOOSE for its availability
        self._poll_time = 0.1

        # Whether or not we called initialize()
        self._initialized = False

        # The file socket we created, if any
        self._file_socket = None

    def __del__(self):
        self.kill()

    def isProcessRunning(self):
        """Returns whether or not a moose process is running"""
        return self._moose_process is not None and self._moose_process.poll() is None

    def possiblyRemoveSocket(self):
        """Attempts to remove the file socket if one was created
        and it exists."""
        if self._file_socket and os.path.exists(self._file_socket):
            try:
                os.remove(self._file_socket)
                self._file_socket = None
            except:
                pass

    def kill(self):
        """Kills the underlying moose process if one is running"""
        if self.isProcessRunning():
            self._moose_process.kill()
            self._moose_process.wait()
        self.possiblyRemoveSocket()

    class ControlException(Exception):
        """Basic exception for an error within the MooseControl"""
        def __init__(self, message):
            super().__init__(message)

    def _requireMooseProcess(self):
        """Throws an exception if the moose process is not running (only if one was spwaned)"""
        if self._moose_process and not self.isProcessRunning():
            raise self.ControlException('The MOOSE process has ended')

    def _requests_wrapper(self, function_name, *args, **kwargs):
        """Helper for wrapping a request function with the name function_name
        that uses a patch for dealing with file socket"""
        if self._file_socket:
            from .requests_unixsocket import Session
            accessor = Session()
        else:
            accessor = requests
        return getattr(accessor, function_name)(*args, **kwargs)

    def _requests_get(self, *args, **kwargs):
        """Wrapper for requests.get that uses a patch for dealing with a file socket"""
        return self._requests_wrapper('get', *args, **kwargs)

    def _requests_post(self, *args, **kwargs):
        """Wrapper for requests.post that uses a patch for dealing with a file socket"""
        return self._requests_wrapper('post', *args, **kwargs)

    def _get(self, path: str):
        """Calls GET on the webserver

        Parameters:
            path (str): The path to call GET on
        Returns:
            int: The HTTP status code
            dict or None: The returned JSON data, if any, otherwise None
        """
        if not self._initialized:
            raise self.ControlException('Attempting GET without calling initialize()')
        self._requireMooseProcess()
        self._requireListening()

        r = self._requests_get(f'{self._url}/{path}')
        r.raise_for_status()

        r_json = None
        if r.headers.get('content-type') == 'application/json':
            r_json = r.json()

        return int(r.status_code), r_json

    def _post(self, path: str, data: dict):
        """Calls POST on the webserver

        Parameters:
            path (str): The path to call GET on
            data (dict): The JSON data to include
        Returns:
            int: The HTTP status code
            dict or None: The returned JSON data, if any, otherwise None
        """
        if not self._initialized:
            raise self.ControlException('Attempting POST without calling initialize()')
        self._requireMooseProcess()
        self._requireListening()
        self._requireWaiting()

        r = self._requests_post(f'{self._url}/{path}', json=data)

        r_json = None
        if r.headers.get('content-type') == 'application/json':
            r_json = r.json()
            error = r_json.get('error')
            if error is not None:
                logger.error(error)
                raise self.ControlException(f'WebServerControl error: {error}')
        r.raise_for_status()

        return r.status_code, r_json

    @staticmethod
    def _checkResponse(expected_keys: list, data: list):
        """Internal helper for checking the keys in data"""
        for key in data.keys():
            if key not in expected_keys:
                raise self.ControlException(f'Unexpected key "{key}"')
        for key in expected_keys:
            if key not in data:
                raise self.ControlException(f'Missing expected key "{key}"')

    def isListening(self) -> bool:
        """Returns whether or not the webserver is listening"""
        try:
            r = self._requests_get(f'{self._url}/check')
        except requests.exceptions.ConnectionError:
            return False
        return r.status_code == 200

    def _requireListening(self):
        """Internal helper that throws if the server is not listening"""
        if not self.isListening():
            raise self.ControlException('MOOSE is not listening')

    def initialize(self):
        """Starts the MOOSE process (if enabled) and waits for
        the MOOSE webserver to start listening

        Must be called before doing any other operations"""
        if self._initialized:
            raise self.ControlException('Already called initialize()')

        # The port to listen on, if any
        port = None

        # MOOSE command is provided; start the process
        if self._moose_command:
            # The command line argument we'll append to set where to listen in the app
            listen_command = None

            # Specify the port command
            if self._moose_port is not None:
                port = int(self._moose_port)
                listen_command = f'Controls/{self._moose_control_name}/port={port}'
            # Specify the socket command, determining a randon socket to connect to
            else:
                suffix = ''.join(random.choices(string.ascii_lowercase + string.digits, k=6))
                self._file_socket = os.path.join(tempfile.gettempdir(), f'moose_control_{suffix}')
                logger.info(f'Determined file socket {self._file_socket} for communication')
                listen_command = f'Controls/{self._moose_control_name}/file_socket={self._file_socket}'

            # Append the listen command
            moose_command = self._moose_command + [listen_command]

            # Spawn the moose process
            logger.info(f'Spawning MOOSE with command "{moose_command}"')
            self._moose_process = self.spawnMoose(moose_command, self._inherit_environment)

            # And setup the threaded reader that will pipe the moose process
            # to the common logger
            def read_process(pipe):
                for line in iter(pipe.readline, ""):
                    logging.getLogger('MooseControl.app').info(line.rstrip())
            self._moose_reader = Thread(target=read_process,
                                        args=[self._moose_process.stdout],
                                        daemon=True)
            self._moose_reader.start()

            # This should be running now
            self._requireMooseProcess()
        # MOOSE command is not provided; just connect via the port
        else:
            logger.info(f'Using provided port {self._moose_port} for communication')
            port = int(self._moose_port)

        # Set the URL for communication
        if port is not None:
            self._url = f'http://localhost:{port}'
        else:
            self._url = f'http+unix://{self._file_socket.replace("/", "%2F")}'

        # Wait for the webserver to listen
        url_clean = self._url.replace('%2F', '/') # cleanup %2F for socket listening
        logger.info(f'Waiting for the webserver to start on "{url_clean}"')
        while True:
            time.sleep(self._poll_time)
            self._requireMooseProcess()
            if self.isListening():
                break

        self._initialized = True
        logger.info(f'Webserver is listening on "{url_clean}"')

    def finalize(self):
        """Waits for the MOOSE webserver to stop listening and for
        the MOOSE process to exit (if one was setup)

        Use this when you think MOOSE should be done. This will
        throw in the event that the webserver is waiting for
        input when you think it should be done"""
        if self._moose_process:
            logger.info('Waiting for the webserver to stop and for the app process to exit')
        else:
            logger.info('Waiting for the webserver to stop')

        webserver_stopped = False
        while True:
            time.sleep(self._poll_time)

            # If the server is no longer responding, we're good
            if not self.isListening():
                if not webserver_stopped:
                    logger.info('Webserver has stopped listening')
                    webserver_stopped = True
                if self._moose_process:
                    self._moose_process.wait()
                    return_code = self._moose_process.returncode
                    logger.info(f'App process has exited with code {return_code}')
                break

            # Make sure that the control isn't waiting for input
            # while we think we should be done, because this will
            # hang forever
            waiting_flag = None
            try:
                waiting_flag = self.getWaitingFlag()
            except: # could be shutting down; can fil
                pass
            if waiting_flag is not None:
                raise self.ControlException(f'Final wait is stuck because the control is waiting on flag {waiting_flag}')

        # Clean this up if it exists
        self.possiblyRemoveSocket()

    def returnCode(self):
        """Gets the return code of the moose process"""
        if self._moose_process is None:
            raise self.ControlException('A MOOSE process was not spawned')
        if self._moose_process.poll() is None:
            raise self.ControlException('The MOOSE process has not completed')
        return self._moose_process.returncode

    def wait(self, flag: str = None) -> str:
        """Waits for the MOOSE webserver and returns once the WebServerControl
        is waiting for input

        Parameters:
            flag (str or None): The expected execute on flag, if any, otherwise None
        Returns:
            str: The execute on flag
        """
        if flag:
            logger.info(f'Waiting for the webserver to be at execute on flag {flag}')
        else:
            logger.info(f'Waiting for the webserver')

        # Poll until we're available
        while True:
            # Wait every so often
            time.sleep(self._poll_time)

            # If the process is provided, die if it is no longer running
            if self._moose_process and self._moose_process.poll() is not None:
                raise self.ControlException(f'Attached MOOSE process has terminated')

            # Wait for it to be available
            current_flag = self.getWaitingFlag()
            if current_flag is None:
                continue

            logger.info(f'Webserver is waiting at execute on flag {current_flag}')
            if flag is not None and current_flag != flag:
                raise self.ControlException(f'Unexpected execute on flag {current_flag}')
            return current_flag

    def getWaitingFlag(self) -> str:
        """Gets the current EXECUTE_ON flag that WebServerControl is waiting on

        Returns:
            str or None: The current EXECUTE_ON flag if waiting, otherwise None
        """
        status, r = self._get('waiting')
        if status != 200:
            raise self.ControlException(f'Unexpected status {status} from waiting')

        if not r['waiting']:
            self._checkResponse(['waiting'], r)
            return None

        self._checkResponse(['waiting', 'execute_on_flag'], r)
        return r['execute_on_flag']

    def isWaiting(self) -> bool:
        """Checks whether or not the webserver is waiting

        Returns:
            bool: Whether or not the webserver is waiting
        """
        return self.getWaitingFlag() is not None

    def _requireWaiting(self):
        """Internal helper that throws if the server is not waiting"""
        if not self.isWaiting():
            raise self.ControlException('MOOSE is not waiting')

    def setContinue(self):
        """Tells the WebServerControl to continue"""
        logger.info(f'Telling the webserver to continue')
        self._requireWaiting()
        status, r_json = self._get('continue')
        if status != 200:
            raise self.ControlException(f'Unexpected status {status} from continue')
        if r_json is not None:
            raise self.ControlException(f'Unexpected data {r_json} from continue')
        logger.debug(f'Successfully told the webserver to continue')

    def _setControllable(self, path: str, type: str, value):
        """Internal helper for setting a controllable value"""
        logger.info(f'Setting controllable value {path}')
        logger.debug(f'Setting controllable {type} value {path}={value}')
        data = {'name': path, 'value': value, 'type': type}
        status, _ = self._post('set/controllable', data)
        if status != 201:
            raise self.ControlException(f'Unexpected status {status} from setting controllable value')
        logger.debug(f'Successfully set controllable value {path}')

    @staticmethod
    def _requireNumeric(value):
        """Helper for requring that the given value is numeric"""
        if not isinstance(value, (int, float)):
            raise self.ControlException(f'Value "{value}" is not numeric')

    @staticmethod
    def _requireType(value, value_type):
        """Helper for requring that the given value is a certain type"""
        if not isinstance(value, value_type):
            raise self.ControlException(f'value is not a {value_type}; is a {type(value)}')

    def setControllableBool(self, path: str, value: bool):
        """Sets a controllable bool-valued parameter

        The provided value must be a bool

        Parameters:
            path (str): The path of the controllable value
            value (float): The value to set
        """
        self._requireType(value, bool)
        self._setControllable(path, 'bool', value)

    def setControllableReal(self, path: str, value: float):
        """Sets a controllable Real-valued parameter

        The provided value must be numeric

        Parameters:
            path (str): The path of the controllable value
            value (float): The value to set
        """
        self._requireNumeric(value)
        self._setControllable(path, 'Real', float(value))

    def setControllableInt(self, path: str, value: int):
        """Sets a controllable int-valued parameter

        The provided value must be numeric

        Parameters:
            path (str): The path of the controllable value
            value (int): The value to set
        """
        self._requireNumeric(value)
        self._setControllable(path, 'int', int(value))

    def setControllableVectorReal(self, path: str, value: list[float]):
        """Sets a controllable vector-of-Real parameter

        The provided value must be a list of numeric values

        Parameters:
            path (str): The path of the controllable value
            value (list): The value to set
        """
        self._requireType(value, list)
        value_list = []
        for entry in value:
            self._requireNumeric(entry)
            value_list.append(entry)
        self._setControllable(path, 'std::vector<Real>', value_list)

    def setControllableVectorInt(self, path: str, value: list[int]):
        """Sets a controllable vector-of-int parameter

        The provided value must be a list of numeric values

        Parameters:
            path (str): The path of the controllable value
            value (list): The value to set
        """
        self._requireType(value, list)
        value_list = []
        for entry in value:
            self._requireNumeric(entry)
            value_list.append(int(entry))
        self._setControllable(path, 'std::vector<int>', value_list)

    def setControllableString(self, path: str, value: str):
        """Sets a controllable string parameter

        Parameters:
            path (str): The path of the controllable value
            value (str): The value to set
        """
        self._setControllable(path, 'std::string', str(value))

    def setControllableVectorString(self, path: str, value: list[float]):
        """Sets a controllable vector-of-string parameter

        The provided value must be a list

        Parameters:
            path (str): The path of the controllable value
            value (list): The value to set
        """
        self._requireType(value, list)
        for i in range(len(value)):
            value[i] = str(value[i])
        self._setControllable(path, 'std::vector<std::string>', value)

    def getPostprocessor(self, name: str) -> float:
        """Gets a postprocessor value

        Parameters:
            name (str): The name of the postprocessor
        Returns:
            float: The value of the postprocessor
        """
        logger.debug(f'Getting postprocessor value for "{name}"')
        self._requireWaiting()

        data = {'name': name}
        status, r = self._post('get/postprocessor', data)

        if status != 200:
            raise self.ControlException(f'Unexpected status {status} from getting postprocessor value')
        self._checkResponse(['value'], r)

        value = float(r['value'])
        logger.debug(f'Successfully retrieved postprocessor value {name}={value}')

        return value

    @staticmethod
    def spawnMoose(cmd: list[str], inherit_environment: bool = True) -> subprocess.Popen:
        """Helper for spawning a MOOSE process that will be cleanly killed"""
        popen_kwargs = {'stdout': subprocess.PIPE,
                        'stderr': subprocess.STDOUT,
                        'text': True,
                        'universal_newlines': True,
                        'bufsize': 1,
                        'env': os.environ if inherit_environment else None,
                        'preexec_fn': os.setsid}

        return subprocess.Popen(cmd, **popen_kwargs)
