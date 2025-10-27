# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test moosecontrol.runners.baserunner.PortRunner."""

# ruff: noqa: E402

import os
from subprocess import PIPE, Popen
from time import sleep
from typing import Tuple

import pytest
from common import (
    BASE_INPUT,
    LIVE_BASERUNNER_KWARGS,
    MooseControlTestCase,
    setup_moose_python_path,
)
from requests import Session

setup_moose_python_path()

from moosecontrol import MooseControl, PortRunner
from moosecontrol.runners.portrunner import DEFAULT_HOST
from test_runners_baserunner import check_baserunner_cleanup_live

DUMMY_PORT = 13579


class TestSubprocessSocketRunner(MooseControlTestCase):
    """Test moosecontrol.runners.portrunner.PortRunner."""

    def test_init(self):
        """Test __init__() with the required arguments."""
        runner = PortRunner(DUMMY_PORT)
        self.assertEqual(runner.port, DUMMY_PORT)
        self.assertEqual(runner.host, DEFAULT_HOST)
        self.assertEqual(runner.url, f"{DEFAULT_HOST}:{DUMMY_PORT}")

    def test_init_host(self):
        """Test __init__() with a host provided."""
        host = "http://foo.bar"
        runner = PortRunner(DUMMY_PORT, host=host)
        self.assertEqual(runner.port, DUMMY_PORT)
        self.assertEqual(runner.host, host)
        self.assertEqual(runner.url, f"{host}:{DUMMY_PORT}")

    def test_init_kwargs(self):
        """Test passing kwargs to the BaseRunner in __init__()."""
        kwargs = {
            "poll_time": 0.001,
            "poke_poll_time": 20.0,
            "initialize_timeout": 10.0,
        }
        runner = PortRunner(DUMMY_PORT, **kwargs)
        for key, value in kwargs.items():
            self.assertEqual(getattr(runner, key), value)

    def test_build_session(self):
        """Test build_session()."""
        session = PortRunner(DUMMY_PORT).build_session()
        self.assertIsInstance(session, Session)
        session.close()

    def test_find_available_port(self):
        """Test find_available_port()."""
        port = PortRunner.find_available_port()
        self.assertTrue(PortRunner.port_is_available(port))

    def setup_live(self) -> Tuple[PortRunner, Popen]:
        """Set up a live test."""
        input_path = os.path.join(self.directory.name, "input.i")
        port = PortRunner.find_available_port()

        # Spawn the MOOSE process
        with open(input_path, "w") as f:
            f.write(BASE_INPUT)
        command = [
            self.get_moose_exe(),
            "-i",
            input_path,
            f"Controls/web_server/port={port}",
            "--color=off",
        ]
        process = Popen(command, stdout=PIPE, stderr=PIPE, text=True)

        # Initialize; wait for connection
        runner = PortRunner(port, **LIVE_BASERUNNER_KWARGS)
        data = MooseControl.required_initialize_data(self)
        runner.initialize(data)
        self.assert_in_log("MOOSE webserver is listening")

        # Input has one continue on INITIAL
        while not runner.get("waiting").data["waiting"]:
            sleep(0.001)

        return runner, process

    @pytest.mark.moose
    def test_live(self):
        """Test running a MOOSE input live."""
        runner, process = self.setup_live()

        # Continue on the one timestep
        runner.get("continue")

        # Finalize; should delete socket
        runner.finalize()

        # Wait for the MOOSE process to finish up
        stdout, _ = process.communicate()

        self.assertEqual(process.returncode, 0)
        self.assertIn("Solve Skipped!", stdout)

    @pytest.mark.moose
    def test_cleanup_live(self):
        """Test cleanup() live, which should kill the process."""
        self.allow_log_warnings = True

        runner, process = self.setup_live()

        # Call cleanup, will kill the process
        runner.cleanup()

        # Capture process output
        _, stderr = process.communicate()

        # Check state versus what the BaseRunner tests say it should be
        check_baserunner_cleanup_live(self, runner, stderr, process.returncode)
