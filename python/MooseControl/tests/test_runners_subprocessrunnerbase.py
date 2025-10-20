#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from unittest.mock import patch

from common import MooseControlTestCase, setup_moose_python_path
setup_moose_python_path()

from MooseControl.runners.subprocessrunnerbase import SubprocessRunnerBase
from MooseControl.runners.utils.subprocessreader import SubprocessReader

COMMAND = ['/path/to/moose-opt', '-i', 'input.i']
ADDITIONAL_COMMAND = ['--color=off']
MOOSE_CONTROL_NAME = 'web_server'
ARGS = {
    'command': COMMAND,
    'moose_control_name': MOOSE_CONTROL_NAME
}

class SubprocessRunnerBaseTest(SubprocessRunnerBase):
    def get_additional_command(self):
        return ADDITIONAL_COMMAND

def patch_runner(name: str, **kwargs):
    """
    Convenience method for patching the SubprocessRunnerBase.
    """
    return patch(f'MooseControl.runners.subprocessrunnerbase.SubprocessRunnerBase.{name}', **kwargs)

class TestSubprocessRunnerBase(MooseControlTestCase):
    """
    Tests MooseControl.runners.subprocessrunnerbase.SubprocessRunnerBase.
    """
    def test_init(self):
        """
        Tests __init__() with the required arguments.
        """
        runner = SubprocessRunnerBaseTest(**ARGS)
        self.assertEqual(runner.command, COMMAND)
        self.assertEqual(runner.moose_control_name, MOOSE_CONTROL_NAME)
        self.assertEqual(runner.directory, os.getcwd())
        self.assertTrue(runner.use_subprocess_reader)

    def test_init_directory(self):
        """
        Tests __init__() with a directory provided.
        """
        runner = SubprocessRunnerBaseTest(**ARGS, directory=self.directory.name)
        self.assertEqual(runner.directory, self.directory.name)

    def test_init_no_use_subprocess_reader(self):
        """
        Tests __init__() with use_subprocess_reader=False.
        """
        runner = SubprocessRunnerBaseTest(**ARGS, use_subprocess_reader=False)
        self.assertFalse(runner.use_subprocess_reader)

    def test_initialize_no_directory(self):
        """
        Tests initialize() when the directory doesn't exist.
        """
        directory = '/foo/bar'
        self.assertFalse(os.path.exists(directory))
        runner = SubprocessRunnerBaseTest(**ARGS, directory=directory)

        regex = f'Directory {directory} does not exist'
        with self.assertRaisesRegex(FileNotFoundError, regex):
            runner.initialize()

    def run_initialize_dummy_process(self, use_subprocess_reader: bool):
        """
        Helper for testing initialize() with a dummy process, with
        and without the reader thread.
        """
        self._caplog.clear()

        runner = SubprocessRunnerBaseTest(
            **ARGS,
            directory=self.directory.name,
            use_subprocess_reader=use_subprocess_reader
        )

        # Form a fake process that does something briefly
        output = ['foo', 'bar']
        command = f'sleep 0.1 && echo "{"\n".join(output)}" && sleep 0.1'
        start_process_before = runner.start_process
        def mock_start_process(*_, **kwargs):
            kwargs['shell'] = True
            return start_process_before(command, **kwargs)

        # Call initialize, ignoring the parent initialize() and using our
        # dummy process
        with patch_runner('start_process', new=mock_start_process):
            runner.initialize()

        # Process should still be running
        pid = runner.get_process_pid()
        self.assertIsNotNone(pid)

        # Wait for it to finish up
        self.assertIsNone(runner._process.poll())
        runner._process.wait()
        if use_subprocess_reader:
            self.assertIsNotNone(runner._subprocess_reader)
            runner._subprocess_reader.join()
        else:
            self.assertIsNone(runner._subprocess_reader)

        # Start and stop log + output if reader thread enabled
        log_size = 2 + ((len(output) + 2) if use_subprocess_reader else 0)
        self.assertLogSize(log_size)

        # Process starting
        self.assertLogMessage(
            0,
            f'Starting MOOSE process with command {command}',
            name='SubprocessRunnerBase'
        )
        # Reader thread started
        if use_subprocess_reader:
            self.assertLogMessage(
                1,
                'Subprocess reader started',
                name='SubprocessReader',
            )
        # Process started
        self.assertLogMessage(
            2 if use_subprocess_reader else 1,
            f'MOOSE process started with pid {pid}',
            name='SubprocessRunnerBase'
        )
        # Process output should be in the log
        if use_subprocess_reader:
            log_i = 1
            for val in output:
                log_i = self.assertInLog(
                    SubprocessReader.OUTPUT_PREFIX + val,
                    name='SubprocessReader',
                    after_index=log_i
                )

    def test_initialize_dummy_process(self):
        """
        Tests initialize() with dummy process with and without the
        reader thread.
        """
        self.run_initialize_dummy_process(True)
        self.run_initialize_dummy_process(False)

    def test_get_full_command(self):
        """
        Tests get_full_command().
        """
        runner = SubprocessRunnerBaseTest(**ARGS)
        full_command = runner.get_full_command()
        self.assertEqual(full_command, runner.command + ADDITIONAL_COMMAND)

    def test_finalize_wait_nothing(self):
        """
        Tests waiting for nothing in finalize()
        """
        runner = SubprocessRunnerBaseTest(**ARGS, use_subprocess_reader=False)
        runner.finalize()
        self.assertLogSize(0)

    def test_finalize_wait_process(self):
        """
        Tests running a dummy process and waiting for it in finalize().
        """
        runner = SubprocessRunnerBaseTest(
            **ARGS,
            directory=self.directory.name,
            use_subprocess_reader=False
        )

        runner._process = runner.start_process('sleep 0.1', shell=True)

        pid = runner.get_process_pid()
        self.assertIsNotNone(pid)

        self._caplog.clear()
        runner.finalize()

        self.assertLogSize(2)
        self.assertLogMessage(0, f'Waiting for MOOSE process {pid} to end...')
        self.assertLogMessage(1, 'MOOSE process has ended')

    def test_finalize_wait_subprocess_reader(self):
        """
        Tests running a dummy reader thread and waiting for it in finalize().
        """
        runner = SubprocessRunnerBaseTest(
            **ARGS,
            directory=self.directory.name,
            use_subprocess_reader=True
        )

        process = runner.start_process('sleep 0.1', shell=True)
        runner._subprocess_reader = SubprocessReader(process)
        runner._subprocess_reader.start()
        runner.finalize()

        self.assertInLog('Waiting for the reader thread to end...', levelname='DEBUG')
        self.assertInLog('Reader thread has ended', levelname='DEBUG')
        self.assertIsNone(runner._subprocess_reader)

        process.wait()

    def test_finalize_wait_subprocess_reader_not_alive(self):
        """
        Tests running a dummy reader thread and not waiting for it in
        finalize() because it has completed.
        """
        runner = SubprocessRunnerBaseTest(
            **ARGS,
            directory=self.directory.name,
            use_subprocess_reader=True
        )

        process = runner.start_process('exit 0', shell=True)
        runner._subprocess_reader = SubprocessReader(process)
        runner._subprocess_reader.start()
        runner._subprocess_reader.join()
        process.wait()

        self._caplog.clear()
        runner.finalize()

        self.assertIsNone(runner._subprocess_reader)
        self.assertLogSize(0)

    def test_cleanup_kill_process(self):
        """
        Tests cleanup() killing a dummy process.
        """
        runner = SubprocessRunnerBaseTest(**ARGS)

        runner._process = runner.start_process('sleep 10', shell=True)
        self.assertTrue(runner.is_process_running())

        self._caplog.clear()
        runner.cleanup()

        self.assertFalse(runner.is_process_running())

        self.assertLogSize(3)
        self.assertLogMessage(0, 'MOOSE process still running on cleanup; killing', levelname='WARNING')

    def test_cleanup_wait_subprocess_reader(self):
        """
        Tests cleanup() waiting for the reader thread to finish.
        """
        runner = SubprocessRunnerBaseTest(**ARGS)

        process = runner.start_process('sleep 0.1', shell=True)
        runner._subprocess_reader = SubprocessReader(process)
        runner._subprocess_reader.start()

        runner.cleanup()

        self.assertFalse(runner._subprocess_reader.is_alive())

        self.assertLogSize(5)
        self.assertLogMessage(2, 'Reader thread still running on cleanup; waiting', levelname='WARNING')
        self.assertLogMessage(4, 'Reader thread has ended')

    def test_kill_process(self):
        """
        Tests kill_process() on a dummy process.
        """
        runner = SubprocessRunnerBaseTest(
            **ARGS,
            directory=self.directory.name,
        )

        runner._process = runner.start_process('sleep 10', shell=True)
        pid = runner.get_process_pid()
        self.assertIsNotNone(pid)

        self._caplog.clear()
        runner.kill_process()

        self.assertFalse(runner.is_process_running())

        self.assertLogSize(2)
        self.assertLogMessage(0, f'Killing MOOSE process {pid}')
        self.assertLogMessage(1, 'Killed MOOSE process has ended')

    def test_kill_process_no_process(self):
        """
        Tests kill_process() when a process doesn't exist.
        """
        runner = SubprocessRunnerBaseTest(
            **ARGS,
            directory=self.directory.name,
        )

        runner.kill_process()
        self.assertLogSize(0)
