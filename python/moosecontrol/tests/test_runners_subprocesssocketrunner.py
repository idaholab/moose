# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test moosecontrol.runners.subprocesssocketrunner.SubprocessSocketRunner."""

# ruff: noqa: E402

import os
from re import match
from tempfile import NamedTemporaryFile, gettempdir
from time import sleep
from unittest.mock import patch

import pytest
from common import (
    BASE_INPUT,
    LIVE_BASERUNNER_KWARGS,
    MooseControlTestCase,
    setup_moose_python_path,
)

setup_moose_python_path()

from moosecontrol import MooseControl, SubprocessSocketRunner
from test_runners_baserunner import check_baserunner_cleanup_live
from test_runners_interfaces_subprocessrunnerinterface import (
    ARGS,
    COMMAND,
    MOOSE_CONTROL_NAME,
    get_process_output,
)

RUNNER = "moosecontrol.SubprocessSocketRunner"
INTERFACE = "moosecontrol.runners.interfaces.SubprocessRunnerInterface"
SOCKET_RUNNER = "moosecontrol.SocketRunner"

FAKE_SOCKET_PATH = "/path/to/foo.sock"


def patch_runner(name: str, **kwargs):
    """Patch a method on the SubprocessRunner."""
    return patch(f"{RUNNER}.{name}", **kwargs)


class TestSubprocessSocketRunner(MooseControlTestCase):
    """Test moosecontrol.runners.subprocesssocketrunner.SubprocessSocketRunner."""

    def test_init(self):
        """Test __init__() with the required arguments."""
        runner = SubprocessSocketRunner(**ARGS)
        self.assertEqual(runner.command, COMMAND)
        self.assertEqual(runner.moose_control_name, MOOSE_CONTROL_NAME)
        self.assertEqual(runner.directory, os.getcwd())
        self.assertEqual(os.path.dirname(runner.socket_path), gettempdir())
        self.assertTrue(runner.use_subprocess_reader)

    def test_init_socket_path(self):
        """Test __init__() with a socket_path provided."""
        runner = SubprocessSocketRunner(**ARGS, socket_path=FAKE_SOCKET_PATH)
        self.assertEqual(runner.socket_path, FAKE_SOCKET_PATH)

    def test_random_socket_path(self):
        """Test random_socket_path()."""
        name = SubprocessSocketRunner.random_socket_path()
        match_re = rf"{gettempdir()}/moosecontrol_[a-z0-9]{{7}}.sock"
        self.assertIsNotNone(match(match_re, name))

    def test_get_additional_command(self):
        """Test get_additional_command()."""
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
        """Test initialize() when the socket already exists."""
        with NamedTemporaryFile() as f:
            runner = SubprocessSocketRunner(**ARGS, socket_path=f.name)
            regex = f"Socket {runner.socket_path} already exists"
            with self.assertRaisesRegex(FileExistsError, regex):
                runner.initialize({})

    def test_initialize(self):
        """Test initialize() calling initialize() on the parents in order."""
        runner = SubprocessSocketRunner(**ARGS)
        methods = [
            RUNNER + ".initialize_start",
            INTERFACE + ".initialize",
            SOCKET_RUNNER + ".initialize",
        ]
        self.assert_methods_called_in_order(methods, lambda: runner.initialize({}))

    def test_finalize(self):
        """Test finalize() calling finalize() on the parents in order."""
        runner = SubprocessSocketRunner(**ARGS)
        methods = [INTERFACE + ".finalize", SOCKET_RUNNER + ".finalize"]
        self.assert_methods_called_in_order(methods, lambda: runner.finalize())

    def test_cleanup(self):
        """Test cleanup() calling cleanup() on the parents in order."""
        runner = SubprocessSocketRunner(**ARGS)
        methods = [SOCKET_RUNNER + ".cleanup", INTERFACE + ".cleanup"]
        self.assert_methods_called_in_order(methods, lambda: runner.cleanup())

    def setup_live(self) -> SubprocessSocketRunner:
        """Set up a live test."""
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

        data = MooseControl.required_initialize_data(self)
        runner.initialize(data)

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
        """Test running a MOOSE input live."""
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
        """Test cleanup() live, which should kill the process."""
        self.allow_log_warnings = True

        runner = self.setup_live()

        # Call cleanup, will kill the process
        runner.cleanup()

        self.assertFalse(runner.is_process_running())

        # Check state versus what the BaseRunner tests say it should be
        output = "\n".join(get_process_output(self, runner))
        returncode = runner.get_return_code()
        check_baserunner_cleanup_live(self, runner, output, returncode)
