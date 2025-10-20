#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from subprocess import Popen, PIPE
from tempfile import NamedTemporaryFile, TemporaryDirectory
from unittest import main, skipUnless
from unittest.mock import patch

from common import BASE_INPUT, MOOSE_EXE, CaptureLogTestCase, \
    mock_response, setup_moose_python_path
setup_moose_python_path()

from moosecontrol import SocketRunner
from moosecontrol.requests_unixsocket import Session

DUMMY_SOCKET_PATH = '/foo/bar.sock'

def patch_runner(name: str, **kwargs):
    """
    Convenience method for patching the SocketRunner.
    """
    return patch(f'moosecontrol.SocketRunner.{name}', **kwargs)

def patch_base(name: str, **kwargs):
    """
    Convenience method for patching the BaseRunner.
    """
    return patch(f'moosecontrol.runners.BaseRunner.{name}', **kwargs)

class TestBaseRunner(CaptureLogTestCase):
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
            'poll_time': 0.001,
            'poke_poll_time': 20.0,
            'initialize_timeout': 10.0
        }
        runner = SocketRunner(DUMMY_SOCKET_PATH, **kwargs)
        for key, value in kwargs.items():
            self.assertEqual(getattr(runner, key), value)

    def test_url(self):
        """
        Tests property url.
        """
        runner = SocketRunner(DUMMY_SOCKET_PATH)
        self.assertEqual(runner.url, 'http+unix://%2Ffoo%2Fbar.sock')

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
        self.assertFalse(SocketRunner.socket_exists('/foo/bar'))

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
            with patch('stat.S_ISSOCK', return_value=True) as patch_issock:
                self.assertTrue(SocketRunner.socket_exists(f.name))
        patch_issock.assert_called_once()

    def test_initialize_immediate(self):
        """
        Tests initialize() when the socket is found immediately,
        ignoring the parent initialize().
        """
        runner = SocketRunner(DUMMY_SOCKET_PATH)
        with patch_base('initialize') as parent_initialize:
            with patch_runner('socket_exists', return_value=True) as socket_exists:
                runner.initialize()

        parent_initialize.assert_called_once()
        socket_exists.assert_called_once()
        self.assertLogSize(1)
        self.assertLogMessage(0, f'Found connection socket {DUMMY_SOCKET_PATH}')

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
        with patch_base('initialize') as parent_initialize:
            with patch_runner('socket_exists', new=mock_socket_exists):
                runner.initialize()

        parent_initialize.assert_called_once()
        self.assertLogSize(2)
        self.assertLogMessage(0, f'Waiting for connection socket {DUMMY_SOCKET_PATH}...')
        self.assertLogMessage(1, f'Found connection socket {DUMMY_SOCKET_PATH}')

    def test_delete_socket(self):
        """
        Tests delete_socket().
        """
        with NamedTemporaryFile() as f:
            runner = SocketRunner(f.name)
            runner.delete_socket()
            self.assertFalse(os.path.exists(f.name))

        self.assertLogSize(1)
        self.assertLogMessage(0, f'Deleting socket {f.name}')

    def test_finalize_delete_socket(self):
        """
        Tests finalize() deleting the socket, ignoring the
        parent finalize()
        """
        with NamedTemporaryFile() as f:
            runner = SocketRunner(f.name)
            with patch_base('finalize') as parent_finalize:
                runner.finalize()
            self.assertFalse(os.path.exists(f.name))

        parent_finalize.assert_called_once()

    def test_cleanup(self):
        """
        Tests cleanup().
        """
        # Socket not used
        with NamedTemporaryFile() as f:
            runner = SocketRunner(f.name)
            with patch_base('cleanup') as base_cleanup:
                with patch_runner('delete_socket') as delete_socket:
                    runner.cleanup()
            base_cleanup.assert_called()
            delete_socket.assert_not_called()

        # Used, path set, doesn't exist
        with NamedTemporaryFile() as f:
            os.remove(f.name)
            runner = SocketRunner(f.name)
            with patch_base('cleanup') as base_cleanup:
                with patch_runner('delete_socket') as delete_socket:
                    runner.cleanup()
            base_cleanup.assert_called()
            delete_socket.assert_not_called()

        # Used, path set, does exist, calls delete_socket()
        with NamedTemporaryFile() as f:
            runner = SocketRunner(f.name)
            runner._socket_used = True
            with patch_base('cleanup') as base_cleanup:
                with patch_runner('delete_socket') as delete_socket:
                    runner.cleanup()
            base_cleanup.assert_called()
            delete_socket.assert_called_once()

    def test_initialize_finalize(self):
        """
        Tests initialize() and finalize() together.
        """
        with NamedTemporaryFile() as f:
            runner = SocketRunner(f.name)

            with patch('moosecontrol.requests_unixsocket.Session.get', return_value=mock_response()):
                with patch_runner('socket_exists', return_value=True):
                    runner.initialize()

            runner.finalize()
            self.assertFalse(os.path.exists(f.name))

    @skipUnless(MOOSE_EXE is not None, 'MOOSE_EXE is not set')
    def test_live(self):
        """
        Tests running a MOOSE input live.
        """
        with TemporaryDirectory() as dir:
            input_path = os.path.join(dir, 'input.i')
            socket_path = os.path.join(dir, 'socket.sock')

            # Spawn the MOOSE process
            with open(input_path, 'w') as f:
                f.write(BASE_INPUT)
            command = [
                MOOSE_EXE,
                '-i',
                input_path,
                f'Controls/web_server/file_socket={socket_path}',
                '--color=off'
            ]
            process = Popen(command, stdout=PIPE, text=True)

            # Initialize; wait for socket and connection
            runner = SocketRunner(socket_path)
            runner.initialize()
            socket_i = self.assertInLog(f'Found connection socket {socket_path}')
            self.assertInLog(f'MOOSE webserver is listening', after_index=socket_i)

            # Input has one continue on INITIAL
            runner.get('continue')

            # Finalize; should delete socket
            runner.finalize()
            self.assertInLog(f'Deleting socket {socket_path}')

            # Wait for the MOOSE process to finish up
            stdout, _ = process.communicate()

            self.assertNoWarningLogs()
            self.assertEqual(process.returncode, 0)
            self.assertIn('Solve Skipped!', stdout)
            self.assertFalse(os.path.exists(socket_path))

if __name__ == '__main__':
    main()
