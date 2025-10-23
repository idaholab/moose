# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test moosecontrol.runners.subprocessportrunner.SubprocessPortRunner."""

# ruff: noqa: E402

import os
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

from moosecontrol import SubprocessPortRunner
from test_runners_baserunner import check_baserunner_cleanup_live
from test_runners_interfaces_subprocessrunnerinterface import (
    ARGS,
    COMMAND,
    MOOSE_CONTROL_NAME,
    get_process_output,
)

FAKE_PORT = 60000

RUNNER = "moosecontrol.SubprocessPortRunner"
INTERFACE = "moosecontrol.runners.interfaces.SubprocessRunnerInterface"
PORT_RUNNER = "moosecontrol.PortRunner"


def patch_runner(name: str, **kwargs):
    """Patch a method on the SubprocessPortRunner."""
    return patch(f"{RUNNER}.{name}", **kwargs)


class TestSubprocessPortRunner(MooseControlTestCase):
    """Test moosecontrol.runners.subprocessportrunner.SubprocessPortRunner."""

    def test_init(self):
        """Test __init__() with the required arguments."""
        runner = SubprocessPortRunner(**ARGS)
        self.assertEqual(runner.command, COMMAND)
        self.assertEqual(runner.moose_control_name, MOOSE_CONTROL_NAME)
        self.assertEqual(runner.directory, os.getcwd())
        self.assertIsInstance(runner.port, int)
        self.assertTrue(runner.use_subprocess_reader)

    def test_init_socket_path(self):
        """Test __init__() with a port provided."""
        runner = SubprocessPortRunner(**ARGS, port=FAKE_PORT)
        self.assertEqual(runner.port, FAKE_PORT)

    def test_get_additional_command(self):
        """Test get_additional_command()."""
        runner = SubprocessPortRunner(**ARGS, port=FAKE_PORT)
        result = runner.get_additional_command()
        self.assertEqual(
            result,
            [f"Controls/{runner.moose_control_name}/port={FAKE_PORT}", "--color=off"],
        )

    def test_initialize_port_unavailable(self):
        """Test initialize() when the port is not available."""
        runner = SubprocessPortRunner(**ARGS, port=FAKE_PORT)
        with patch_runner("port_is_available", return_value=False):
            regex = f"Port {FAKE_PORT} is already used"
            with self.assertRaisesRegex(ConnectionRefusedError, regex):
                runner.initialize()

    def test_initialize(self):
        """Tests initialize() calling initialize() on the parents in order."""
        runner = SubprocessPortRunner(**ARGS)
        methods = [
            RUNNER + ".initialize_start",
            INTERFACE + ".initialize",
            PORT_RUNNER + ".initialize",
        ]
        self.assert_methods_called_in_order(methods, lambda: runner.initialize())

    def test_finalize(self):
        """Test finalize() calling finalize() on the parents in order."""
        runner = SubprocessPortRunner(**ARGS)
        methods = [INTERFACE + ".finalize", PORT_RUNNER + ".finalize"]
        self.assert_methods_called_in_order(methods, lambda: runner.finalize())

    def test_cleanup(self):
        """Test cleanup() calling cleanup() on the parents in order."""
        runner = SubprocessPortRunner(**ARGS)
        methods = [PORT_RUNNER + ".cleanup", INTERFACE + ".cleanup"]
        self.assert_methods_called_in_order(methods, lambda: runner.cleanup())

    def setup_live(self) -> SubprocessPortRunner:
        """Set up a live test."""
        input_file = os.path.join(self.directory.name, "input.i")
        with open(input_file, "w") as f:
            f.write(BASE_INPUT)

        command = [self.get_moose_exe(), "-i", input_file]
        runner = SubprocessPortRunner(
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
        output = get_process_output(self, runner)
        returncode = runner.get_return_code()
        check_baserunner_cleanup_live(self, runner, output, returncode)
