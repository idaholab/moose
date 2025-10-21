#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: disable=logging-fstring-interpolation

import os
from abc import ABC, abstractmethod
from logging import getLogger
from subprocess import Popen, PIPE, STDOUT
from typing import Optional

from moosecontrol.runners.utils import SubprocessReader

logger = getLogger('SubprocessRunnerBase')

DEFAULT_DIRECTORY = os.getcwd()

class SubprocessRunnerBase(ABC):
    """
    Base class for subprocess runners.

    Is an abstract base class and is used in
    SubprocessSocketRunner and SubprocessPortRunner.
    """
    def __init__(self,
                 command: list[str],
                 moose_control_name: str,
                 directory: str = DEFAULT_DIRECTORY,
                 use_subprocess_reader: bool = True):
        """
        Parameters
        ----------
        command : list[str]
            The command to spawn the subprocess.
        moose_control_name : str
            The name of the WebServerControl in input.

        Optional Parameters
        -------------------
        directory : str
            Directory to run in. Defaults to the current
            working directory.
        use_subprocess_reader : bool
            Whether or not to spawn a separate reader thread
            to collect subprocess output. Defaults to true.

        """
        assert isinstance(command, list)
        for v in command:
            assert isinstance(v, str)
        assert isinstance(moose_control_name, str)
        assert isinstance(use_subprocess_reader, bool)

        # The command to run to start the MOOSE process
        self._command: list[str] = command
        # The name of the WebServerControl in MOOSE input
        self._moose_control_name: str = moose_control_name
        # The directory to run in
        self._directory: str = str(os.path.abspath(directory))
        # Whether or not to use the reader thread
        self._use_subprocess_reader: bool = use_subprocess_reader

        # The underlying process
        self._process: Optional[Popen] = None
        # The thread that reads from the process output
        self._subprocess_reader: Optional[SubprocessReader] = None

    @property
    def command(self) -> list[str]:
        """
        Get the command to run.
        """
        return self._command

    @property
    def moose_control_name(self) -> str:
        """
        The name of the WebServerControl object in MOOSE input.
        """
        return self._moose_control_name

    @property
    def directory(self) -> str:
        """
        The directory to run in.
        """
        return self._directory

    @property
    def use_subprocess_reader(self) -> bool:
        """
        Whether or not to use the reader thread.
        """
        return self._use_subprocess_reader

    def initialize(self):
        """
        Initializes the subprocess and optionally the reader thread.

        Should be called first in derived classes within initialize().
        """
        if not os.path.isdir(self.directory):
            raise FileNotFoundError(f'Directory {self.directory} does not exist')

        # Start the process
        self._process = self.start_process(self.get_full_command())

        # Start the reader thread if enabled
        if self.use_subprocess_reader:
            self._subprocess_reader = SubprocessReader(self._process)
            self._subprocess_reader.start()

        # Report the PID
        pid = self.get_pid()
        assert pid is not None
        logger.info(f'MOOSE process started with pid {pid}')

    def finalize(self):
        """
        Finalizes the subprocess (waits for it to end) and optionally
        waits for the reader thread to end if it is running.

        Should be called first in derived classes within finalize().
        """
        pid = self.get_pid()
        if pid is not None:
            assert self._process is not None
            logger.info(f'Waiting for MOOSE process {pid} to end...')
            self._process.wait()
            logger.info('MOOSE process has ended')

        if self._subprocess_reader is not None:
            if self._subprocess_reader.is_alive():
                logger.debug('Waiting for the reader thread to end...')
                self._subprocess_reader.join()
                logger.debug('Reader thread has ended')
            self._subprocess_reader = None

    def cleanup(self):
        """
        Performs cleanup.

        Should ideally do nothing unless something went wrong.
        """
        if hasattr(self, '_process') and self.is_process_running():
            logger.warning('MOOSE process still running on cleanup; killing')
            self.kill_process()

        subprocess_reader = getattr(self, '_subprocess_reader', None)
        if subprocess_reader is not None and subprocess_reader.is_alive():
            logger.warning('Reader thread still running on cleanup; waiting')
            subprocess_reader.join()
            logger.info('Reader thread has ended')

    @abstractmethod
    def get_additional_command(self) -> list[str]:
        """
        Get the additional arguments to start the subprocess with.

        Must be overridden.
        """
        raise NotImplementedError # pragma: no cover

    def get_full_command(self) -> list[str]:
        """
        Get the full command to start the process with.

        Is the user's command plus any additional commands
        needed by the derived class.
        """
        return self.command + self.get_additional_command()

    @staticmethod
    def start_process(command: list[str], **kwargs) -> Popen:
        """
        Starts a process with the given command.
        """
        logger.info(f'Starting MOOSE process with command {command}')

        kwargs.update({
            'stdout': PIPE,
            'stderr': STDOUT,
            'text': True,
            'universal_newlines': True,
            'bufsize': 1,
            'preexec_fn': os.setsid
        })
        return Popen(command, **kwargs)

    def get_pid(self) -> Optional[int]:
        """
        Get the PID of the underlying process if it is running.
        """
        return self._process.pid if self.is_process_running() else None # type: ignore

    def is_process_running(self) -> bool:
        """
        Whether or not the underlying process is running.
        """
        return self._process is not None and self._process.poll() is None

    def kill_process(self):
        """
        Kills the running process, if any.
        """
        pid = self.get_pid()
        if self._process is not None and (pid := self.get_pid()) is not None:
            logger.info(f'Killing MOOSE process {pid}')
            self._process.kill()
            self._process.wait()
            logger.info('Killed MOOSE process has ended')

    def get_return_code(self) -> int:
        """
        Gets the return code of the process that ran.
        """
        assert self._process is not None
        if self._process.poll() is None:
            raise RuntimeError('The process is still running')
        return self._process.returncode
