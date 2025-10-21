#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: skip-file
# type: ignore

import os
from unittest import skipUnless
from unittest.mock import patch

from common import BASE_INPUT, MOOSE_EXE, MooseControlTestCase, \
    setup_moose_python_path
setup_moose_python_path()

from moosecontrol import SubprocessPortRunner
from moosecontrol.runners.utils import SubprocessReader

from test_runners_subprocessrunnerbase import ARGS, COMMAND, MOOSE_CONTROL_NAME

FAKE_PORT = 60000

RUNNER = 'moosecontrol.SubprocessPortRunner'
RUNNER_BASE = 'moosecontrol.runners.subprocessrunnerbase.SubprocessRunnerBase'
PORT_RUNNER = 'moosecontrol.PortRunner'

def patch_runner(name: str, **kwargs):
    """
    Convenience method for patching the SubprocessPortRunner.
    """
    return patch(f'{RUNNER}.{name}', **kwargs)

class TestSubprocessPortRunner(MooseControlTestCase):
    """
    Tests moosecontrol.runners.subprocessportrunner.SubprocessPortRunner.
    """
    def test_init(self):
        """
        Tests __init__() with the required arguments.
        """
        runner = SubprocessPortRunner(**ARGS)
        self.assertEqual(runner.command, COMMAND)
        self.assertEqual(runner.moose_control_name, MOOSE_CONTROL_NAME)
        self.assertEqual(runner.directory, os.getcwd())
        self.assertIsInstance(runner.port, int)
        self.assertTrue(runner.use_subprocess_reader)

    def test_init_socket_path(self):
        """
        Tests __init__() with a port provided.
        """
        runner = SubprocessPortRunner(**ARGS, port=FAKE_PORT)
        self.assertEqual(runner.port, FAKE_PORT)

    def test_get_additional_command(self):
        """
        Tests get_additional_command()
        """
        runner = SubprocessPortRunner(**ARGS, port=FAKE_PORT)
        result = runner.get_additional_command()
        self.assertEqual(
            result,
            [
                f'Controls/{runner.moose_control_name}/port={FAKE_PORT}',
                '--color=off'
            ]
        )

    def test_initialize_port_unavailable(self):
        """
        Tests initialize() when the port is not available.
        """
        runner = SubprocessPortRunner(**ARGS, port=FAKE_PORT)
        with patch_runner('port_is_available', return_value=False):
            regex = f'Port {FAKE_PORT} is already used'
            with self.assertRaisesRegex(ConnectionRefusedError, regex):
                runner.initialize()

    def test_initialize(self):
        """
        Tests initialize(), which should call initialize() on the parent
        SubprocessRunnerBase and PortRunner.
        """
        runner = SubprocessPortRunner(**ARGS)
        methods = [
            RUNNER + '.initialize_start',
            RUNNER_BASE + '.initialize',
            PORT_RUNNER + '.initialize'
        ]
        self.assert_methods_called_in_order(methods, lambda: runner.initialize())

    def test_finalize(self):
        """
        Tests finalize(), which should call finalize() on the parent
        SubprocessRunnerBase and PortRunner.
        """
        runner = SubprocessPortRunner(**ARGS)
        methods = [RUNNER_BASE + '.finalize', PORT_RUNNER + '.finalize']
        self.assert_methods_called_in_order(methods, lambda: runner.finalize())

    def test_cleanup(self):
        """
        Tests cleanup(), which should call cleanup() on the parent
        SubprocessRunnerBase and PortRunner.
        """
        runner = SubprocessPortRunner(**ARGS)
        methods = [RUNNER_BASE + '.cleanup', PORT_RUNNER + '.cleanup']
        self.assert_methods_called_in_order(methods, lambda: runner.cleanup())

    @skipUnless(MOOSE_EXE is not None, 'MOOSE_EXE is not set')
    def test_live(self):
        """
        Tests running a MOOSE input live.
        """
        input_file = os.path.join(self.directory.name, 'input.i')
        with open(input_file, 'w') as f:
            f.write(BASE_INPUT)

        command = [MOOSE_EXE, '-i', input_file]
        runner = SubprocessPortRunner(
            command=command,
            moose_control_name='web_server',
            directory=self.directory.name
        )
        full_command = runner.get_full_command()

        runner.initialize()
        pid = runner.get_pid()
        self.assertIsNotNone(pid)
        self.assert_in_log(f'Starting MOOSE process with command {full_command}')
        self.assert_in_log(f'MOOSE process started with pid {pid}')

        self.assertTrue(runner.is_process_running())
        runner.get('continue')

        runner.finalize()

        self.assert_no_warning_logs()
        self.assertFalse(runner.is_process_running())

        process_output = [v.message for v in self._caplog.records if v.name == 'SubprocessReader']
        self.assertIn(SubprocessReader.OUTPUT_PREFIX + ' Solve Skipped!', process_output)
        self.assertEqual(runner._process.returncode, 0)
