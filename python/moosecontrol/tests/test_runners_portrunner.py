#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from requests import Session
from subprocess import Popen, PIPE
from tempfile import TemporaryDirectory
from unittest import main, skipUnless
from unittest.mock import patch

from common import BASE_INPUT, MOOSE_EXE, MooseControlTestCase, setup_moose_python_path
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
        self.assertEqual(runner.url, f'{DEFAULT_HOST}:{DUMMY_PORT}')

    def test_init_host(self):
        """
        Tests __init__() with a host provided.
        """
        host = 'http://foo.bar'
        runner = PortRunner(DUMMY_PORT, host=host)
        self.assertEqual(runner.port, DUMMY_PORT)
        self.assertEqual(runner.host, host)
        self.assertEqual(runner.url, f'{host}:{DUMMY_PORT}')

    def test_init_kwargs(self):
        """
        Tests passing kwargs to the BaseRunner in __init__().
        """
        kwargs = {
            'poll_time': 0.001,
            'poke_poll_time': 20.0,
            'initialize_timeout': 10.0
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

    @skipUnless(MOOSE_EXE is not None, 'MOOSE_EXE is not set')
    def test_live(self):
        """
        Tests running a MOOSE input live.
        """
        with TemporaryDirectory() as dir:
            input_path = os.path.join(dir, 'input.i')
            port = PortRunner.find_available_port()

            # Spawn the MOOSE process
            with open(input_path, 'w') as f:
                f.write(BASE_INPUT)
            command = [
                MOOSE_EXE,
                '-i',
                input_path,
                f'Controls/web_server/port={port}',
                '--color=off'
            ]
            process = Popen(command, stdout=PIPE, text=True)

            # Initialize; wait for connection
            runner = PortRunner(port)
            runner.initialize()
            self.assertInLog('MOOSE webserver is listening')

            # Input has one continue on INITIAL
            runner.get('continue')

            # Finalize; should delete socket
            runner.finalize()

            # Wait for the MOOSE process to finish up
            stdout, _ = process.communicate()

            self.assertNoWarningLogs()
            self.assertEqual(process.returncode, 0)
            self.assertIn('Solve Skipped!', stdout)

if __name__ == '__main__':
    main()
