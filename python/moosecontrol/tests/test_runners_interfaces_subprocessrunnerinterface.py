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
import subprocess
from unittest.mock import patch, MagicMock, PropertyMock

from common import MooseControlTestCase, setup_moose_python_path

setup_moose_python_path()

from moosecontrol.runners.interfaces import SubprocessRunnerInterface
from moosecontrol.runners.utils import SubprocessReader

COMMAND = ["/path/to/moose-opt", "-i", "input.i"]
ADDITIONAL_COMMAND = ["--color=off"]
MOOSE_CONTROL_NAME = "web_server"
ARGS = {"command": COMMAND, "moose_control_name": MOOSE_CONTROL_NAME}


class SubprocessRunnerInterfaceTest(SubprocessRunnerInterface):
    def get_additional_command(self):
        return ADDITIONAL_COMMAND


def patch_runner(name: str, **kwargs):
    """
    Convenience method for patching the SubprocessRunnerInterface.
    """
    return patch(
        f"moosecontrol.runners.interfaces.SubprocessRunnerInterface.{name}",
        **kwargs,
    )


def get_process_output(
    test: MooseControlTestCase, runner: SubprocessRunnerInterface
) -> list[str]:
    """
    Helper for reading the process output from the SubprocessReader.
    """
    return [
        v.message.replace(SubprocessReader.OUTPUT_PREFIX, "", 1)
        for v in test._caplog.records
        if v.name == "SubprocessReader"
    ]


class TestSubprocessRunnerInterface(MooseControlTestCase):
    """
    Tests moosecontrol.runners.interfaces.subprocessrunnerinterface.SubprocessRunnerInterface.
    """

    def test_init(self):
        """
        Tests __init__() with the required arguments.
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS)
        self.assertEqual(runner.command, COMMAND)
        self.assertEqual(runner.moose_control_name, MOOSE_CONTROL_NAME)
        self.assertEqual(runner.directory, os.getcwd())
        self.assertTrue(runner.use_subprocess_reader)

    def test_init_directory(self):
        """
        Tests __init__() with a directory provided.
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS, directory=self.directory.name)
        self.assertEqual(runner.directory, self.directory.name)

    def test_init_no_use_subprocess_reader(self):
        """
        Tests __init__() with use_subprocess_reader=False.
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS, use_subprocess_reader=False)
        self.assertFalse(runner.use_subprocess_reader)

    def test_initialize_no_directory(self):
        """
        Tests initialize() when the directory doesn't exist.
        """
        directory = "/foo/bar"
        self.assertFalse(os.path.exists(directory))
        runner = SubprocessRunnerInterfaceTest(**ARGS, directory=directory)

        regex = f"Directory {directory} does not exist"
        with self.assertRaisesRegex(FileNotFoundError, regex):
            runner.initialize()

    def run_initialize_dummy_process(self, use_subprocess_reader: bool):
        """
        Helper for testing initialize() with a dummy process, with
        and without the reader thread.
        """
        self._caplog.clear()

        runner = SubprocessRunnerInterfaceTest(
            **ARGS,
            directory=self.directory.name,
            use_subprocess_reader=use_subprocess_reader,
        )

        # Form a fake process that does something briefly
        output = ["foo", "bar"]
        joined_output = "\n".join(output)
        command = ["echo", joined_output]
        start_process_before = runner.start_process

        def mock_start_process(*_, **kwargs):
            return start_process_before(command, **kwargs)

        # Call initialize, ignoring the parent initialize() and using our
        # dummy process
        with patch_runner("start_process", new=mock_start_process):
            runner.initialize()

        # Wait for it to finish up
        runner._process.wait()
        if use_subprocess_reader:
            self.assertIsNotNone(runner._subprocess_reader)
            runner._subprocess_reader.join()
        else:
            self.assertIsNone(runner._subprocess_reader)
        pid = runner.get_pid()
        self.assertIsNotNone(pid)

        # Start and stop log + output if reader thread enabled
        log_size = 2 + ((len(output) + 2) if use_subprocess_reader else 0)
        self.assert_log_size(log_size)

        # Process starting
        self.assert_log_message(
            0,
            f"Starting MOOSE process with command {command}",
            name="SubprocessRunnerInterface",
        )
        # Reader thread started
        if use_subprocess_reader:
            self.assert_log_message(
                1,
                "Subprocess reader started",
                name="SubprocessReader",
            )
        # Process started
        self.assert_log_message(
            2 if use_subprocess_reader else 1,
            f"MOOSE process started with pid {pid}",
            name="SubprocessRunnerInterface",
        )
        # Process output should be in the log
        if use_subprocess_reader:
            log_i = 1
            for val in output:
                log_i = self.assert_in_log(
                    SubprocessReader.OUTPUT_PREFIX + val,
                    name="SubprocessReader",
                    after_index=log_i,
                )

    def test_initialize_dummy_process_with_reader_thread(self):
        """
        Tests initialize() with dummy process with the reader thread.
        """
        self.run_initialize_dummy_process(True)

    def test_initialize_dummy_process_without_reader_thread(self):
        """
        Tests initialize() with dummy process with the reader thread.
        """
        self.run_initialize_dummy_process(False)

    def test_initialize_process_directory(self):
        """
        Tests that initialize() uses the directory as the running
        directory for the process.
        """
        self._caplog.clear()

        runner = SubprocessRunnerInterfaceTest(
            **ARGS,
            directory=self.directory.name,
            use_subprocess_reader=False,
        )

        start_process_before = runner.start_process

        def mock_start_process(*_, **kwargs):
            return start_process_before(["pwd"], **kwargs)

        with patch_runner("start_process", new=mock_start_process):
            runner.initialize()
        stdout, _ = runner._process.communicate()

        self.assertEqual(stdout, f"{self.directory.name}\n")

    def test_get_full_command(self):
        """
        Tests get_full_command().
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS)
        full_command = runner.get_full_command()
        self.assertEqual(full_command, runner.command + ADDITIONAL_COMMAND)

    def test_finalize_wait_nothing(self):
        """
        Tests waiting for nothing in finalize()
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS, use_subprocess_reader=False)
        runner.finalize()
        self.assert_log_size(0)

    def test_finalize_wait_process(self):
        """
        Tests waiting for a dummy process in finalize().
        """
        runner = SubprocessRunnerInterfaceTest(
            **ARGS, directory=self.directory.name, use_subprocess_reader=False
        )

        pid = 1234
        process = MagicMock()
        type(process).pid = PropertyMock(return_value=pid)
        process.wait.return_value = None
        runner._process = process
        runner.is_process_running = MagicMock(return_value=True)

        runner.finalize()

        self.assertEqual(pid, runner.get_pid())
        runner.is_process_running.assert_called_once()
        process.wait.assert_called_once()
        self.assert_log_size(2)
        self.assert_log_message(0, f"Waiting for MOOSE process {pid} to end...")
        self.assert_log_message(1, "MOOSE process has ended")

    def test_finalize_wait_subprocess_reader(self):
        """
        Tests running a dummy reader thread and waiting for it in finalize().
        """
        runner = SubprocessRunnerInterfaceTest(
            **ARGS, directory=self.directory.name, use_subprocess_reader=True
        )

        reader = MagicMock()
        reader.is_alive.return_value = True
        reader.join.return_value = None
        runner._subprocess_reader = reader

        runner.finalize()

        reader.is_alive.assert_called_once()
        reader.join.assert_called_once()
        self.assert_log_size(2)
        self.assert_log_message(
            0, "Waiting for the reader thread to end...", levelname="DEBUG"
        )
        self.assert_log_message(1, "Reader thread has ended", levelname="DEBUG")
        self.assertIsNone(runner._subprocess_reader)

    def test_finalize_wait_subprocess_reader_not_alive(self):
        """
        Tests running a dummy reader thread and not waiting for it in
        finalize() because it has completed.
        """
        runner = SubprocessRunnerInterfaceTest(
            **ARGS, directory=self.directory.name, use_subprocess_reader=True
        )

        reader = MagicMock()
        reader.is_alive.return_value = False
        reader.join.return_value = None
        runner._subprocess_reader = reader

        runner.finalize()

        reader.is_alive.assert_called_once()
        reader.join.assert_not_called()
        self.assertIsNone(runner._subprocess_reader)
        self.assert_log_size(0)

    def test_cleanup_kill_process(self):
        """
        Tests cleanup() killing a dummy process.
        """
        self.allow_log_warnings = True

        runner = SubprocessRunnerInterfaceTest(**ARGS)

        runner._process = runner.start_process(["sleep", "0.1"])
        self.assertTrue(runner.is_process_running())

        self._caplog.clear()
        runner.cleanup()

        self.assertFalse(runner.is_process_running())

        self.assert_log_size(3)
        self.assert_log_message(
            0, "MOOSE process still running on cleanup; killing", levelname="WARNING"
        )

    def test_cleanup_wait_subprocess_reader(self):
        """
        Tests cleanup() waiting for the reader thread to finish.
        """
        self.allow_log_warnings = True

        runner = SubprocessRunnerInterfaceTest(**ARGS)

        reader = MagicMock()
        reader.is_alive.return_value = True
        reader.join.return_value = None
        runner._subprocess_reader = reader

        runner.cleanup()

        reader.is_alive.assert_called_once()
        reader.join.assert_called_once()
        self.assert_log_size(2)
        self.assert_log_message(
            0, "Reader thread still running on cleanup; waiting", levelname="WARNING"
        )
        self.assert_log_message(1, "Reader thread has ended")

    def test_start_process(self):
        """
        Tests start_process().
        """
        message = "foo"
        command = ["echo", message]
        with patch("subprocess.Popen", wraps=subprocess.Popen) as mock_popen:
            process = SubprocessRunnerInterface.start_process(command)
        stdout, _ = process.communicate()

        mock_popen.assert_called_once_with(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            universal_newlines=True,
            bufsize=1,
            preexec_fn=os.setsid,
        )
        self.assertEqual(stdout, f"{message}\n")
        self.assert_log_size(1)
        self.assert_log_message(0, f"Starting MOOSE process with command {command}")

    def test_start_process_kwargs(self):
        """
        Tests that start_process() passes its kwargs to popen.
        """
        with patch("subprocess.Popen", wraps=subprocess.Popen) as mock_popen:
            process = SubprocessRunnerInterface.start_process(
                ["echo", "foo"], cwd=self.directory.name
            )
        process.communicate()

        _, kwargs = mock_popen.call_args
        self.assertEqual(kwargs["cwd"], self.directory.name)

    def test_get_pid(self):
        """
        Tests get_pid().
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS)
        runner._process = runner.start_process(["/bin/sh", "-c", "exit 0"])
        self.assertEqual(runner.get_pid(), runner._process.pid)
        runner._process.wait()

    def test_get_pid_no_process(self):
        """
        Tests get_pid() when there is no process.
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS)
        self.assertIsNone(runner._process)
        self.assertIsNone(runner.get_pid())

    def test_is_process_running(self):
        """
        Tests is_process_running().
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS, directory=self.directory.name)
        runner._process = runner.start_process(["sleep", "0.01"], shell=True)
        self.assertTrue(runner.is_process_running())
        runner._process.wait()
        self.assertFalse(runner.is_process_running())

    def test_is_process_running_no_process(self):
        """
        Tests is_process_running() when there is no process.
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS)
        self.assertIsNone(runner._process)
        self.assertFalse(runner.is_process_running())

    def test_kill_process(self):
        """
        Tests kill_process() on a dummy process.
        """
        runner = SubprocessRunnerInterfaceTest(
            **ARGS,
            directory=self.directory.name,
        )

        runner._process = runner.start_process(["sleep", "10"])
        pid = runner.get_pid()
        self.assertIsNotNone(pid)

        self._caplog.clear()
        runner.kill_process()

        self.assertFalse(runner.is_process_running())

        self.assert_log_size(2)
        self.assert_log_message(0, f"Killing MOOSE process {pid}")
        self.assert_log_message(1, "Killed MOOSE process has ended")

    def test_kill_process_no_process(self):
        """
        Tests kill_process() when a process doesn't exist.
        """
        runner = SubprocessRunnerInterfaceTest(
            **ARGS,
            directory=self.directory.name,
        )

        runner.kill_process()
        self.assert_log_size(0)

    def test_get_return_code(self):
        """
        Tests get_return_code().
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS, use_subprocess_reader=False)

        runner._process = runner.start_process(["/bin/sh", "-c", "exit 0"])
        runner._process.wait()

        self.assertEqual(runner._process.returncode, 0)
        self.assertEqual(runner.get_return_code(), 0)

    def test_get_return_code_running(self):
        """
        Tests get_return_code().
        """
        runner = SubprocessRunnerInterfaceTest(**ARGS, use_subprocess_reader=False)

        runner._process = runner.start_process(["sleep", "0.1"])

        with self.assertRaises(RuntimeError) as e:
            runner.get_return_code()
        runner._process.kill()
        self.assertEqual(str(e.exception), "The process is still running")
        runner._process.wait()
