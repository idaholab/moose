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

class MooseControl:
    """Helper for interacting with the MOOSE WebServerControl.

    This object is tested primarily by
    test/tests/controls/web_server_control in the framework."""

    def __init__(self, port: int):
        # The URL we interact with
        self._url = f'http://localhost:{port}'

        # How often we want to poll MOOSE for its availability
        self._poll_time = 0.1

        # Whether or not we called initialWait()
        self._did_initial_wait = False

    def log(self, msg):
        """Helper for printing to a log

        Allows us to easily implement a better log in the future
        if we want to"""
        print(f'[{self.__class__.__name__}]: {msg}')

    class Exception(Exception):
        """Basic exception for an error within the MooseControl"""
        def __init__(self, message):
            super().__init__(message)

    def _get(self, path: str):
        """Calls GET on the webserver

        Parameters:
            path (str): The path to call GET on
        Returns:
            int: The HTTP status code
            dict or None: The returned JSON data, if any, otherwise None
        """
        self.log(f'Getting "{path}"')

        if not self._did_initial_wait:
            raise Exception('Attempting to GET without calling initialWait()')

        r = requests.get(f'{self._url}/{path}')
        self.log(f'Status from "{path}": {r.status_code}')
        r.raise_for_status()

        r_json = None
        if r.headers.get('content-type') == 'application/json':
            r_json = r.json()
            self.log(f'Result from "{path}": {r_json}')

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
        self.log(f'Sending {data} to "{path}"')

        if not self._did_initial_wait:
            raise Exception('Attempting to GET without calling initialWait()')
        self._requireWaiting()

        r = requests.post(f'{self._url}/{path}', json=data)

        r_json = None
        if r.headers.get('content-type') == 'application/json':
            r_json = r.json()
            if 'error' in r.json():
                self.exception(f'HTTP error: "{r.json()["error"]}"')
        r.raise_for_status()

        return r.status_code, r_json

    @staticmethod
    def _checkResponse(expected_keys, data):
        """Internal helper for checking the keys in data"""
        for key in data.keys():
            if key not in expected_keys:
                raise MooseControl.Exception(f'Unexpected key "{key}"')
        for key in expected_keys:
            if key not in data:
                raise MooseControl.Exception(f'Missing expected key "{key}"')

    def initialWait(self):
        """Waits for the MOOSE webserver to start listening

        Must be called before doing any other operations"""
        self.log(f'Waiting for the webserver to start at "{self._url}"')

        if self._did_initial_wait:
            raise Exception('Already called initialWait()')

        while True:
            time.sleep(self._poll_time)
            try:
                requests.get(self._url)
            except requests.exceptions.ConnectionError:
                continue
            break

        self._did_initial_wait = True
        self.log(f'Webserver is listening at "{self._url}"')

    def wait(self, flag=None):
        """Waits for the MOOSE webserver and returns once the WebServerControl
        is waiting for input

        Parameters:
            flag (str or None): The expected execute on flag, if any, otherwise None
        """

        if flag:
            self.log(f'Waiting to be at execute on flag "{flag}"')
        else:
            self.log(f'Waiting to be available')

        # Poll until we're available
        while True:
            # Wait every so often
            time.sleep(self._poll_time)

            # Wait for it to be available
            current_flag = self.getWaitingFlag()
            if current_flag is None:
                continue

            self.log(f'Waiting at execute on flag "{current_flag}"')
            if flag is not None and current_flag != flag:
                raise self.Exception(f'Unexpected execute on flag "{current_flag}"')
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
            raise Exception('MOOSE is not waiting')

    def setContinue(self):
        """Tells the WebServerControl to continue"""
        self.log(f'Telling to continue')
        self._requireWaiting()
        status, r_json = self._get('continue')
        if status != 200:
            raise Exception(f'Unexpected status {status} from continue')
        if r_json is not None:
            raise Exception(f'Unexpected data {r_json} from continue')

    def waitDone(self):
        """Waits for the WebServerControl to be done (no longer responding)"""
        self.log(f'Waiting for completion')
        while True:
            try:
                requests.get(f'{self._url}')
            except requests.exceptions.ConnectionError:
                return

    def setControllableReal(self, path: str, value: float):
        """Sets a controllable Real-valued parameter

        Parameters:
            path (str): The path of the controllable value
            value (float): The value to set
        """
        self.log(f'Setting controllable Real value {path}={value}')
        data = {'name': path, 'value': value}
        status, _ = self._post('set/controllable', data)
        if status != 201:
            raise Exception(f'Unexpected status {status} from setting controllable value')

    def getPostprocessor(self, name: str) -> float:
        """Gets a postprocessor value

        Parameters:
            name (str): The name of the postprocessor
        Returns:
            float: The value of the postprocessor
        """
        self.log(f'Getting postprocessor value for "{name}"')
        self._requireWaiting()

        data = {'name': name}
        status, r = self._post('get/postprocessor', data)

        if status != 200:
            raise Exception(f'Unexpected status {status} from getting postprocessor value')
        self._checkResponse(['value'], r)

        value = float(r['value'])
        self.log(f'Retrieved postprocessor value {name}={value}')

        return value

    @staticmethod
    def getAvailablePort() -> int:
        """Gets an available port on the system to use for the web server"""
        sock = socket.socket()
        sock.bind(('', 0))
        return int(sock.getsockname()[1])

    @staticmethod
    def spawnMoose(cmd: str, cwd) -> subprocess.Popen:
        """Helper for spawning a MOOSE process that will be cleanly killed"""
        popen_kwargs = {'stdout': subprocess.PIPE,
                        'stderr': subprocess.PIPE,
                        'cwd': cwd,
                        'close_fds': False,
                        'shell': True,
                        'text': True}

        # This controls the process being killed properly with the parent
        if platform.system() == "Windows":
            popen_kwargs['creationflags'] = subprocess.CREATE_NEW_PROCESS_GROUP
        else:
            popen_kwargs['preexec_fn'] = os.setsid

        return subprocess.Popen(cmd, **popen_kwargs)
