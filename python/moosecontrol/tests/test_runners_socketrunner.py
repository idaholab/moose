# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
from pathlib import Path
from subprocess import Popen, PIPE
from tempfile import NamedTemporaryFile
from time import sleep
from unittest.mock import patch

import pytest

from common import (
    BASE_INPUT,
    LIVE_BASERUNNER_KWARGS,
    MooseControlTestCase,
    mock_response,
    setup_moose_python_path,
)

setup_moose_python_path()

from moosecontrol import SocketRunner
from moosecontrol.requests_unixsocket import Session

DUMMY_SOCKET_PATH = "/foo/bar.sock"


def patch_runner(name: str, **kwargs):
    """
    Convenience method for patching the SocketRunner.
    """
    return patch(f"moosecontrol.SocketRunner.{name}", **kwargs)


def patch_base(name: str, **kwargs):
    """
    Convenience method for patching the BaseRunner.
    """
    return patch(f"moosecontrol.runners.BaseRunner.{name}", **kwargs)


class TestSocketRunner(MooseControlTestCase):
    """
    Tests moosecontrol.runners.socketrunner.SocketRunner.
    """

    def test_init_no_socket_path(self):
        """
        Tests __init__() with a None socket_path.
        """
        runner = SocketRunner(DUMMY_SOCKET_PATH)
        self.assertEqual(runner.socket_path, DUMMY_SOCKET_PATH)

    def test_init_kwargs(self):
        """
        Tests __init__() passing kwargs to the BaseRunner.
        """
        kwargs = {
            "poll_time": 0.001,
            "poke_poll_time": 20.0,
            "initialize_timeout": 10.0,
        }
        runner = SocketRunner(DUMMY_SOCKET_PATH, **kwargs)
        for key, value in kwargs.items():
            self.assertEqual(getattr(runner, key), value)

    def test_url(self):
        """
        Tests property url.
        """
        runner = SocketRunner(DUMMY_SOCKET_PATH)
        self.assertEqual(runner.url, "http+unix://%2Ffoo%2Fbar.sock")

    def test_session(self):
        """
        Tests build_session().
        """
        runner = SocketRunner(DUMMY_SOCKET_PATH)
        session = runner.build_session()
        self.assertIsInstance(session, Session)
        session.close()

    def test_socket_exists_no_file(self):
        """
        Tests socket_exists() when the file doesn't exist.
        """
        self.assertFalse(SocketRunner.socket_exists("/foo/bar"))

    def test_socket_exists_not_a_socket(self):
        """
        Tests socket_exists() when a file isn't a socket.
        """
        with NamedTemporaryFile() as f:
            with self.assertRaises(FileNotFoundError):
                SocketRunner.socket_exists(f.name)

    def test_socket_exists(self):
        """
        Tests socket_exists() when the socket exists (mocked).
        """
        with NamedTemporaryFile() as f:
            with patch("stat.S_ISSOCK", return_value=True) as patch_issock:
                self.assertTrue(SocketRunner.socket_exists(f.name))
        patch_issock.assert_called_once()

    def test_initialize_immediate(self):
        """
        Tests initialize() when the socket is found immediately,
        ignoring the parent initialize().
        """
        runner = SocketRunner(DUMMY_SOCKET_PATH)
        with patch_base("initialize") as parent_initialize:
            with patch_runner("socket_exists", return_value=True) as socket_exists:
                runner.initialize()

        parent_initialize.assert_called_once()
        socket_exists.assert_called_once()
        self.assert_log_size(1)
        self.assert_log_message(0, f"Found connection socket {DUMMY_SOCKET_PATH}")

    def test_initialize_eventually(self):
        """
        Tests initialize() when the socket is found after some time,
        ignoring the parent initialize().
        """
        exists_calls = 0
        exists_after_calls = 2

        def mock_socket_exists(*args, **kwargs):
            nonlocal exists_calls
            exists_calls += 1
            return exists_after_calls == exists_calls

        runner = SocketRunner(DUMMY_SOCKET_PATH)
        with patch_base("initialize") as parent_initialize:
            with patch_runner("socket_exists", new=mock_socket_exists):
                runner.initialize()

        parent_initialize.assert_called_once()
        self.assert_log_size(2)
        self.assert_log_message(
            0, f"Waiting for connection socket {DUMMY_SOCKET_PATH}..."
        )
        self.assert_log_message(1, f"Found connection socket {DUMMY_SOCKET_PATH}")

    def test_delete_socket(self):
        """
        Tests delete_socket().
        """
        socket = os.path.join(self.directory.name, "sock.sock")
        Path(socket).touch()
        runner = SocketRunner(socket)
        runner.delete_socket()
        self.assertFalse(os.path.exists(socket))

        self.assert_log_size(1)
        self.assert_log_message(0, f"Deleting socket {socket}")

    def test_finalize_delete_socket(self):
        """
        Tests finalize() deleting the socket, ignoring the
        parent finalize()
        """
        socket = os.path.join(self.directory.name, "sock.sock")
        Path(socket).touch()
        runner = SocketRunner(socket)
        with patch_base("finalize") as parent_finalize:
            runner.finalize()
        self.assertFalse(os.path.exists(socket))

        parent_finalize.assert_called_once()

    def test_cleanup_socket_not_used(self):
        """
        Tests cleanup() when the socket is not used.
        """
        socket = os.path.join(self.directory.name, "sock.sock")
        Path(socket).touch()
        runner = SocketRunner(socket)
        with patch_base("cleanup") as base_cleanup:
            with patch_runner("delete_socket") as delete_socket:
                runner.cleanup()
        self.assertTrue(os.path.exists(socket))
        base_cleanup.assert_called()
        delete_socket.assert_not_called()

    def test_cleanup_socket_not_exist(self):
        """
        Tests cleanup() when the socket does not exist.
        """
        socket = os.path.join(self.directory.name, "sock.sock")
        self.assertFalse(os.path.exists(socket))
        runner = SocketRunner(socket)
        with patch_base("cleanup") as base_cleanup:
            with patch_runner("delete_socket") as delete_socket:
                runner.cleanup()
        base_cleanup.assert_called()
        delete_socket.assert_not_called()

    def test_cleanup_socket_delete(self):
        """
        Tests cleanup() when the socket exists and is used,
        which deletes it.
        """
        socket = os.path.join(self.directory.name, "sock.sock")
        Path(socket).touch()
        self.assertTrue(os.path.exists(socket))
        runner = SocketRunner(socket)
        runner._socket_used = True
        with patch_base("cleanup") as base_cleanup:
            with patch_runner("delete_socket") as delete_socket:
                runner.cleanup()
        self.assert_log_size(1)
        self.assert_log_message(
            0, "Socket still exists on cleanup; deleting", levelname="WARNING"
        )
        base_cleanup.assert_called_once()
        delete_socket.assert_called_once()

    def test_initialize_finalize(self):
        """
        Tests initialize() and finalize() together.
        """
        socket = os.path.join(self.directory.name, "sock.sock")
        Path(socket).touch()
        runner = SocketRunner(socket)

        with patch(
            "moosecontrol.requests_unixsocket.Session.get", return_value=mock_response()
        ):
            with patch_runner("socket_exists", return_value=True):
                runner.initialize()

        runner.finalize()
        self.assertFalse(os.path.exists(socket))

    @pytest.mark.moose
    def test_live(self):
        """
        Tests running a MOOSE input live.
        """
        input_path = os.path.join(self.directory.name, "input.i")
        socket_path = os.path.join(self.directory.name, "socket.sock")

        # Spawn the MOOSE process
        with open(input_path, "w") as f:
            f.write(BASE_INPUT)
        command = [
            self.get_moose_exe(),
            "-i",
            input_path,
            f"Controls/web_server/file_socket={socket_path}",
            "--color=off",
        ]
        process = Popen(command, stdout=PIPE, text=True)

        # Initialize; wait for socket and connection
        runner = SocketRunner(socket_path, **LIVE_BASERUNNER_KWARGS)
        runner.initialize()
        socket_i = self.assert_in_log(f"Found connection socket {socket_path}")
        self.assert_in_log(f"MOOSE webserver is listening", after_index=socket_i)

        # Input has one continue on INITIAL
        while not runner.get("waiting").data["waiting"]:
            sleep(0.001)
        runner.get("continue")

        # Finalize; should delete socket
        runner.finalize()
        self.assert_in_log(f"Deleting socket {socket_path}")

        # Wait for the MOOSE process to finish up
        stdout, _ = process.communicate()

        self.assert_no_warning_logs()
        self.assertEqual(process.returncode, 0)
        self.assertIn("Solve Skipped!", stdout)
        self.assertFalse(os.path.exists(socket_path))
