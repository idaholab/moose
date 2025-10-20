#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from re import match
from unittest import main, skipUnless
from unittest.mock import patch
from tempfile import NamedTemporaryFile

from common import BASE_INPUT, MOOSE_EXE, MooseControlTestCase, \
    setup_moose_python_path
setup_moose_python_path()

from moosecontrol import SubprocessSocketRunner
from moosecontrol.runners.utils import SubprocessReader

from test_runners_subprocessrunnerbase import ARGS, COMMAND, MOOSE_CONTROL_NAME

RUNNER = 'moosecontrol.SubprocessSocketRunner'
RUNNER_BASE = 'moosecontrol.runners.subprocessrunnerbase.SubprocessRunnerBase'
SOCKET_RUNNER = 'moosecontrol.SocketRunner'

FAKE_SOCKET_PATH = '/path/to/foo.sock'

def patch_runner(name: str, **kwargs):
    """
    Convenience method for patching the SubprocessPortRunner.
    """
    return patch(f'{RUNNER}.{name}', **kwargs)

class TestSubprocessSocketRunner(MooseControlTestCase):
    """
    Tests moosecontrol.runners.subprocesssocketrunner.SubprocessSocketRunner.
    """
    def test_init(self):
        """
        Tests __init__() with the required arguments.
        """
        runner = SubprocessSocketRunner(**ARGS)
        self.assertEqual(runner.command, COMMAND)
        self.assertEqual(runner.moose_control_name, MOOSE_CONTROL_NAME)
        self.assertEqual(runner.directory, os.getcwd())
        self.assertEqual(os.path.dirname(runner.socket_path), runner.directory)
        self.assertTrue(runner.use_subprocess_reader)

    def test_init_socket_path(self):
        """
        Tests __init__() with a socket_path provided.
        """
        runner = SubprocessSocketRunner(**ARGS, socket_path=FAKE_SOCKET_PATH)
        self.assertEqual(runner.socket_path, FAKE_SOCKET_PATH)

    def test_random_socket_name(self):
        """
        Tests random_socket_name().
        """
        name = SubprocessSocketRunner.random_socket_name()
        self.assertIsNotNone(match(r'[a-z0-9]{5}', name))

    def test_get_additional_command(self):
        """
        Tests get_additional_command()
        """
        runner = SubprocessSocketRunner(**ARGS, socket_path=FAKE_SOCKET_PATH)
        result = runner.get_additional_command()
        self.assertEqual(
            result,
            [
                f'Controls/{runner.moose_control_name}/file_socket={FAKE_SOCKET_PATH}',
                '--color=off'
            ]
        )

    def test_initialize_socket_exists(self):
        """
        Tests initialize() when the socket already exists.
        """
        with NamedTemporaryFile() as f:
            runner = SubprocessSocketRunner(**ARGS, socket_path=f.name)
            regex = f'Socket {runner.socket_path} already exists'
            with self.assertRaisesRegex(FileExistsError, regex):
                runner.initialize()

    def test_initialize(self):
        """
        Tests initialize(), which should call initialize() on the parent
        SubprocessRunnerBase and SocketRunner.
        """
        runner = SubprocessSocketRunner(**ARGS)
        methods = [
            RUNNER + '.initialize_start',
            RUNNER_BASE + '.initialize',
            SOCKET_RUNNER + '.initialize'
        ]
        self.assertMethodsCalledInOrder(methods, lambda: runner.initialize())

    def test_finalize(self):
        """
        Tests finalize(), which should call finalize() on the parent
        SubprocessRunnerBase and SocketRunner.
        """
        runner = SubprocessSocketRunner(**ARGS)
        methods = [RUNNER_BASE + '.finalize', SOCKET_RUNNER + '.finalize']
        self.assertMethodsCalledInOrder(methods, lambda: runner.finalize())

    def test_cleanup(self):
        """
        Tests cleanup(), which should call cleanup() on the parent
        SubprocessRunnerBase and SocketRunner.
        """
        runner = SubprocessSocketRunner(**ARGS)
        methods = [RUNNER_BASE + '.cleanup', SOCKET_RUNNER + '.cleanup']
        self.assertMethodsCalledInOrder(methods, lambda: runner.cleanup())

    @skipUnless(MOOSE_EXE is not None, 'MOOSE_EXE is not set')
    def test_live(self):
        """
        Tests running a MOOSE input live.
        """
        input_file = os.path.join(self.directory.name, 'input.i')
        with open(input_file, 'w') as f:
            f.write(BASE_INPUT)

        command = [MOOSE_EXE, '-i', input_file]
        runner = SubprocessSocketRunner(
            command=command,
            moose_control_name='web_server',
            directory=self.directory.name
        )
        full_command = runner.get_full_command()

        runner.initialize()
        pid = runner.get_process_pid()
        self.assertIsNotNone(pid)
        self.assertInLog(f'Starting MOOSE process with command {full_command}')
        self.assertInLog(f'MOOSE process started with pid {pid}')

        self.assertTrue(runner.is_process_running())
        runner.get('continue')

        runner.finalize()

        self.assertNoWarningLogs()
        self.assertFalse(runner.is_process_running())

        process_output = [v.message for v in self._caplog.records if v.name == 'SubprocessReader']
        self.assertIn(SubprocessReader.OUTPUT_PREFIX + ' Solve Skipped!', process_output)
        self.assertEqual(runner._process.returncode, 0)

if __name__ == '__main__':
    main()
