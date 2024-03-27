#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import platform
import requests
import os
import socket
import subprocess
import time
import logging
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

    def __init__(self, moose_command=None, moose_port=None, moose_control_name=None):
        """Constructor

        You must specify either "moose_port" or "moose_command" and "moose_control_name".

        If "moose_port" is specified: Connect to the webserver at this port and
        do not spawn a moose process.

        If "moose_command" is specified: Spawn a moose process and then connect to it
        at an available port. "moose_control_name" must be specified so that the port
        for the WebServerControl object can be set via this class, that is, via the
        command line option Controls/<moose_control_name/port=<port>, where <port>
        is the determined available port.

        Parameters:
            moose_command (str): The command to use to start the moose process
            moose_port (int): The webserver port to connect to
            moose_control_name (str): The name of the input control object
        """
        # Setup a basic logger
        logging.basicConfig(level=logging.INFO,
                            handlers=[logging.StreamHandler()],
                            format='%(levelname)s:%(name)s: %(message)s')

        # Sanity checks on input
        has_moose_command = moose_command is not None
        has_moose_port = moose_port is not None
        has_moose_control_name = moose_control_name is not None
        if not has_moose_command and not has_moose_port:
            raise ValueError('One of "moose_command" or "moose_port" must be provided')
        if has_moose_command and has_moose_port:
            raise ValueError('"moose_command" and "moose_port" cannot be provided together')
        if has_moose_command and not has_moose_control_name:
            raise ValueError('"moose_control_name" must be specified with "moose_command"')
        if not has_moose_control_name and has_moose_port:
            raise ValueError('"moose_control_name" is unused with "moose_port"')

        # Store inputs
        self._moose_command = moose_command
        self._moose_port = moose_port
        self._moose_control_name = moose_control_name

        # Set defaults
        self._url = None
        self._moose_process = None
        self._moose_reader = None

        # How often we want to poll MOOSE for its availability
        self._poll_time = 0.1

        # Whether or not we called initialize()
        self._initialized = False

    def __del__(self):
        self.kill()

    def isProcessRunning(self):
        """Returns whether or not a moose process is running"""
        return self._moose_process is not None and self._moose_process.poll() is None

    def kill(self):
        """Kills the underlying moose process if one is running"""
        if self.isProcessRunning():
            self._moose_process.kill()
            self._moose_process = None

    class Exception(Exception):
        """Basic exception for an error within the MooseControl"""
        def __init__(self, message):
            super().__init__(message)

    def _requireMooseProcess(self):
        """Throws an exception if the moose process is not running (only if one was spwaned)"""
        if self._moose_process and not self.isProcessRunning():
            raise Exception('The MOOSE process has ended')

    def _get(self, path: str):
        """Calls GET on the webserver

        Parameters:
            path (str): The path to call GET on
        Returns:
            int: The HTTP status code
            dict or None: The returned JSON data, if any, otherwise None
        """
        if not self._initialized:
            raise Exception('Attempting GET without calling initialize()')
        self._requireMooseProcess()
        self._requireListening()

        r = requests.get(f'{self._url}/{path}')
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
            raise Exception('Attempting POST without calling initialize()')
        self._requireMooseProcess()
        self._requireListening()
        self._requireWaiting()

        r = requests.post(f'{self._url}/{path}', json=data)

        r_json = None
        if r.headers.get('content-type') == 'application/json':
            r_json = r.json()
            error = r_json.get('error')
            if error is not None:
                logger.error(error)
                raise MooseControl.Exception(f'WebServerControl error: {error}')
        r.raise_for_status()

        return r.status_code, r_json

    @staticmethod
    def _checkResponse(expected_keys: list, data: list):
        """Internal helper for checking the keys in data"""
        for key in data.keys():
            if key not in expected_keys:
                raise MooseControl.Exception(f'Unexpected key "{key}"')
        for key in expected_keys:
            if key not in data:
                raise MooseControl.Exception(f'Missing expected key "{key}"')

    def isListening(self) -> bool:
        """Returns whether or not the webserver is listening"""
        try:
            r = requests.get(f'{self._url}/check')
        except requests.exceptions.ConnectionError:
            return False
        return r.status_code == 200

    def _requireListening(self):
        """Internal helper that throws if the server is not listening"""
        if not self.isListening():
            raise self.Exception('MOOSE is not listening')

    def initialize(self):
        """Starts the MOOSE process (if enabled) and waits for
        the MOOSE webserver to start listening

        Must be called before doing any other operations"""
        if self._initialized:
            raise Exception('Already called initialize()')

        # The port we've decided on
        port = None

        # MOOSE command is provided; start the process
        if self._moose_command:
            # Setup the port moose will run on
            sock = socket.socket()
            sock.bind(('', 0))
            port = int(sock.getsockname()[1])
            sock.close()
            logger.info(f'Determined port {port} for communication')

            # Build the command, including what port to run the WebServerControl on
            moose_command = f"{self._moose_command} Controls/{self._moose_control_name}/port={port}"

            # Spawn the moose process
            logger.info(f'Spawning MOOSE with command "{moose_command}"')
            self._moose_process = self.spawnMoose(moose_command)

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
        self._url = f'http://localhost:{port}'

        # Wait for the webserver to listen
        logger.info(f'Waiting for the webserver to start on "{self._url}"...')
        while True:
            time.sleep(self._poll_time)
            self._requireMooseProcess()
            if self.isListening():
                break

        self._initialized = True
        logger.info(f'Webserver is listening on "{self._url}"')

    def finalize(self):
        """Waits for the MOOSE webserver to stop listening and for
        the MOOSE process to exit (if one was setup)

        Use this when you think MOOSE should be done. This will
        throw in the event that the webserver is waiting for
        input when you think it should be done"""
        if self._moose_process:
            logger.info('Waiting for the webserver to stop and for the app process to exit...')
        else:
            logger.info('Waiting for the webserver to stop...')

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
                return

            # Make sure that the control isn't waiting for input
            # while we think we should be done, because this will
            # hang forever
            waiting_flag = None
            try:
                waiting_flag = self.getWaitingFlag()
            except: # could be shutting down; can fil
                pass
            if waiting_flag is not None:
                raise self.Exception(f'Final wait is stuck because the control is waiting on flag {waiting_flag}')

    def returnCode(self):
        """Gets the return code of the moose process"""
        if self._moose_process is None:
            raise Exception('A MOOSE process was not spawned')
        if self._moose_process.poll() is None:
            raise Exception('The MOOSE process has not completed')
        return self._moose_process.returncode

    def wait(self, flag: str = None):
        """Waits for the MOOSE webserver and returns once the WebServerControl
        is waiting for input

        Parameters:
            flag (str or None): The expected execute on flag, if any, otherwise None
        """

        if flag:
            logger.info(f'Waiting for the webserver to be at execute on flag {flag}...')
        else:
            logger.info(f'Waiting for the webserver...')

        # Poll until we're available
        while True:
            # Wait every so often
            time.sleep(self._poll_time)

            # If the process is provided, die if it is no longer running
            if self._moose_process and self._moose_process.poll() is not None:
                raise self.Exception(f'Attached MOOSE process has terminated')

            # Wait for it to be available
            current_flag = self.getWaitingFlag()
            if current_flag is None:
                continue

            logger.info(f'Webserver is waiting at execute on flag {current_flag}')
            if flag is not None and current_flag != flag:
                raise self.Exception(f'Unexpected execute on flag {current_flag}')
            return

    def getWaitingFlag(self) -> str:
        """Gets the current EXECUTE_ON flag that WebServerControl is waiting on

        Returns:
            str or None: The current EXECUTE_ON flag if waiting, otherwise None
        """
        status, r = self._get('waiting')
        if status != 200:
            raise Exception(f'Unexpected status {status} from waiting')

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
            raise self.Exception('MOOSE is not waiting')

    def setContinue(self):
        """Tells the WebServerControl to continue"""
        logger.info(f'Telling the webserver to continue...')
        self._requireWaiting()
        status, r_json = self._get('continue')
        if status != 200:
            raise Exception(f'Unexpected status {status} from continue')
        if r_json is not None:
            raise Exception(f'Unexpected data {r_json} from continue')
        logger.info(f'Successfully told the webserver to continue')

    def _setControllable(self, path: str, type: str, value):
        """Internal helper for setting a controllable value"""
        logger.info(f'Setting controllable {type} value {path}={value}...')
        data = {'name': path, 'value': value, 'type': type}
        status, _ = self._post('set/controllable', data)
        if status != 201:
            raise Exception(f'Unexpected status {status} from setting controllable value')
        logger.info(f'Successfully set controllable value {path}')

    @staticmethod
    def _requireNumeric(value):
        """Helper for requring that the given value is numeric"""
        if not isinstance(value, (int, float)):
            raise Exception(f'Value "{value}" is not numeric')

    @staticmethod
    def _requireType(value, value_type):
        """Helper for requring that the given value is a certain type"""
        if not isinstance(value, value_type):
            raise MooseControl.Exception(f'value is not a {value_type}; is a {type(value)}')

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
        logger.info(f'Getting postprocessor value for "{name}"...')
        self._requireWaiting()

        data = {'name': name}
        status, r = self._post('get/postprocessor', data)

        if status != 200:
            raise Exception(f'Unexpected status {status} from getting postprocessor value')
        self._checkResponse(['value'], r)

        value = float(r['value'])
        logger.info(f'Successfully retrieved postprocessor value {name}={value}')

        return value

    @staticmethod
    def spawnMoose(cmd: str) -> subprocess.Popen:
        """Helper for spawning a MOOSE process that will be cleanly killed"""
        popen_kwargs = {'stdout': subprocess.PIPE,
                        'stderr': subprocess.STDOUT,
                        'close_fds': False,
                        'shell': True,
                        'text': True,
                        'universal_newlines': True,
                        'bufsize': 1}

        # This controls the process being killed properly with the parent
        if platform.system() == "Windows":
            popen_kwargs['creationflags'] = subprocess.CREATE_NEW_PROCESS_GROUP
        else:
            popen_kwargs['preexec_fn'] = os.setsid

        return subprocess.Popen(cmd, **popen_kwargs)
