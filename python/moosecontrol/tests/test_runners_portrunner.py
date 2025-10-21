# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
from requests import Session
from subprocess import Popen, PIPE
from time import sleep

import pytest

from common import (
    BASE_INPUT,
    LIVE_BASERUNNER_KWARGS,
    MooseControlTestCase,
    setup_moose_python_path,
)

setup_moose_python_path()

from moosecontrol import PortRunner
from moosecontrol.runners.portrunner import DEFAULT_HOST

DUMMY_PORT = 13579


class TestSubprocessSocketRunner(MooseControlTestCase):
    """
    Tests moosecontrol.runners.portrunner.PortRunner.
    """

    def test_init(self):
        """
        Tests __init__() with the required arguments.
        """
        runner = PortRunner(DUMMY_PORT)
        self.assertEqual(runner.port, DUMMY_PORT)
        self.assertEqual(runner.host, DEFAULT_HOST)
        self.assertEqual(runner.url, f"{DEFAULT_HOST}:{DUMMY_PORT}")

    def test_init_host(self):
        """
        Tests __init__() with a host provided.
        """
        host = "http://foo.bar"
        runner = PortRunner(DUMMY_PORT, host=host)
        self.assertEqual(runner.port, DUMMY_PORT)
        self.assertEqual(runner.host, host)
        self.assertEqual(runner.url, f"{host}:{DUMMY_PORT}")

    def test_init_kwargs(self):
        """
        Tests passing kwargs to the BaseRunner in __init__().
        """
        kwargs = {
            "poll_time": 0.001,
            "poke_poll_time": 20.0,
            "initialize_timeout": 10.0,
        }
        runner = PortRunner(DUMMY_PORT, **kwargs)
        for key, value in kwargs.items():
            self.assertEqual(getattr(runner, key), value)

    def test_build_session(self):
        """
        Tests build_session().
        """
        session = PortRunner(DUMMY_PORT).build_session()
        self.assertIsInstance(session, Session)
        session.close()

    def test_find_available_port(self):
        """
        Tests find_available_port().
        """
        port = PortRunner.find_available_port()
        self.assertTrue(PortRunner.port_is_available(port))

    @pytest.mark.moose
    def test_live(self):
        """
        Tests running a MOOSE input live.
        """
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
        process = Popen(command, stdout=PIPE, text=True)

        # Initialize; wait for connection
        runner = PortRunner(port, **LIVE_BASERUNNER_KWARGS)
        runner.initialize()
        self.assert_in_log("MOOSE webserver is listening")

        # Input has one continue on INITIAL
        while not runner.get("waiting").data["waiting"]:
            sleep(0.001)
        runner.get("continue")

        # Finalize; should delete socket
        runner.finalize()

        # Wait for the MOOSE process to finish up
        stdout, _ = process.communicate()

        self.assert_no_warning_logs()
        self.assertEqual(process.returncode, 0)
        self.assertIn("Solve Skipped!", stdout)
