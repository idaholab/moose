# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

# ruff: noqa: E402

import os
from re import match
from unittest.mock import patch
from tempfile import NamedTemporaryFile, gettempdir
from time import sleep

import pytest

from common import (
    BASE_INPUT,
    LIVE_BASERUNNER_KWARGS,
    MooseControlTestCase,
    setup_moose_python_path,
)

setup_moose_python_path()

from moosecontrol import SubprocessSocketRunner

from test_runners_baserunner import check_baserunner_cleanup_live
from test_runners_subprocessrunnerbase import (
    ARGS,
    COMMAND,
    MOOSE_CONTROL_NAME,
    get_process_output,
)

RUNNER = "moosecontrol.SubprocessSocketRunner"
RUNNER_BASE = "moosecontrol.runners.subprocessrunnerbase.SubprocessRunnerBase"
SOCKET_RUNNER = "moosecontrol.SocketRunner"

FAKE_SOCKET_PATH = "/path/to/foo.sock"


def patch_runner(name: str, **kwargs):
    """
    Convenience method for patching the SubprocessPortRunner.
    """
    return patch(f"{RUNNER}.{name}", **kwargs)


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
        self.assertEqual(os.path.dirname(runner.socket_path), gettempdir())
        self.assertTrue(runner.use_subprocess_reader)

    def test_init_socket_path(self):
        """
        Tests __init__() with a socket_path provided.
        """
        runner = SubprocessSocketRunner(**ARGS, socket_path=FAKE_SOCKET_PATH)
        self.assertEqual(runner.socket_path, FAKE_SOCKET_PATH)

    def test_random_socket_path(self):
        """
        Tests random_socket_path().
        """
        name = SubprocessSocketRunner.random_socket_path()
        match_re = rf"{gettempdir()}/moosecontrol_[a-z0-9]{{7}}.sock"
        self.assertIsNotNone(match(match_re, name))

    def test_get_additional_command(self):
        """
        Tests get_additional_command()
        """
        runner = SubprocessSocketRunner(**ARGS, socket_path=FAKE_SOCKET_PATH)
        result = runner.get_additional_command()
        self.assertEqual(
            result,
            [
                f'Controls/{runner.moose_control_name}/file_socket="{FAKE_SOCKET_PATH}"',
                "--color=off",
            ],
        )

    def test_initialize_socket_exists(self):
        """
        Tests initialize() when the socket already exists.
        """
        with NamedTemporaryFile() as f:
            runner = SubprocessSocketRunner(**ARGS, socket_path=f.name)
            regex = f"Socket {runner.socket_path} already exists"
            with self.assertRaisesRegex(FileExistsError, regex):
                runner.initialize()

    def test_initialize(self):
        """
        Tests initialize(), which should call initialize() on the parent
        SubprocessRunnerBase and SocketRunner.
        """
        runner = SubprocessSocketRunner(**ARGS)
        methods = [
            RUNNER + ".initialize_start",
            RUNNER_BASE + ".initialize",
            SOCKET_RUNNER + ".initialize",
        ]
        self.assert_methods_called_in_order(methods, lambda: runner.initialize())

    def test_finalize(self):
        """
        Tests finalize(), which should call finalize() on the parent
        SubprocessRunnerBase and SocketRunner.
        """
        runner = SubprocessSocketRunner(**ARGS)
        methods = [RUNNER_BASE + ".finalize", SOCKET_RUNNER + ".finalize"]
        self.assert_methods_called_in_order(methods, lambda: runner.finalize())

    def test_cleanup(self):
        """
        Tests cleanup(), which should call cleanup() on the parent
        SubprocessRunnerBase and SocketRunner.
        """
        runner = SubprocessSocketRunner(**ARGS)
        methods = [
            SOCKET_RUNNER + ".cleanup",
            RUNNER_BASE + ".cleanup",
        ]
        self.assert_methods_called_in_order(methods, lambda: runner.cleanup())

    def setup_live(self) -> SubprocessSocketRunner:
        """
        Sets up a live test.
        """
        input_file = os.path.join(self.directory.name, "input.i")
        with open(input_file, "w") as f:
            f.write(BASE_INPUT)

        command = [self.get_moose_exe(), "-i", input_file]
        runner = SubprocessSocketRunner(
            command=command,
            directory=self.directory.name,
            moose_control_name="web_server",
            **LIVE_BASERUNNER_KWARGS,
        )
        full_command = runner.get_full_command()

        runner.initialize()
        pid = runner.get_pid()
        self.assertIsNotNone(pid)
        self.assert_in_log(f"Starting MOOSE process with command {full_command}")
        self.assert_in_log(f"MOOSE process started with pid {pid}")

        self.assertTrue(runner.is_process_running())
        while not runner.get("waiting").data["waiting"]:
            sleep(0.001)

        return runner

    @pytest.mark.moose
    def test_live(self):
        """
        Tests running a MOOSE input live.
        """
        runner = self.setup_live()

        # Continue on the one timestep
        runner.get("continue")

        # Wait for process to finish
        runner.finalize()

        self.assertFalse(runner.is_process_running())
        process_output = get_process_output(self, runner)
        self.assertIn(" Solve Skipped!", process_output)
        self.assertEqual(runner.get_return_code(), 0)

    @pytest.mark.moose
    def test_cleanup_live(self):
        """
        Tests cleanup() live, which should kill the process.
        """
        self.allow_log_warnings = True

        runner = self.setup_live()

        # Call cleanup, will kill the process
        runner.cleanup()

        self.assertFalse(runner.is_process_running())

        # Check state versus what the BaseRunner tests say it should be
        output = get_process_output(self, runner)
        returncode = runner.get_return_code()
        check_baserunner_cleanup_live(self, runner, output, returncode)
