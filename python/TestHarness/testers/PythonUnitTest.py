# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from RunApp import RunApp
import os


class PythonUnitTest(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()

        # Input is optional in the base class. Make it required here
        params.addRequiredParam("input", "The python input file to use for this test.")
        params.addParam(
            "test_case",
            "The specific test case to run (Default: All test cases in the module)",
        )
        params.addParam(
            "buffer", False, "Equivalent to passing -b or --buffer to the unittest."
        )
        params.addParam(
            "separate", False, "Run each test in the file in a separate subprocess"
        )
        # We don't want to check for any errors on the screen with unit tests
        params["errors"] = []
        params["valgrind"] = "NONE"
        params["recover"] = False
        # Multiple executions; doesn't support perf graph capture
        params["capture_perf_graph"] = False
        params["restep"] = False

        # Force the use of a single MPI slot by default; -p isn't currently
        # passed to the unit test so if the user really wants multiple ranks
        # they better force it via min_parallel. --n-threads *is* passed
        # through (see augmentEnvironment below), but is still forced to a
        # single thread here by default; subclasses that make use of it
        # (e.g. MMSTest) can raise this default back up.
        params["max_parallel"] = 1
        params["max_threads"] = 1

        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

        # Because the unittest doesn't actually use -p from the test harness,
        # it should run with exactly one parallel option. For example, if
        # your unittest manually runs with mpiexec -n 2, this job shouldn't
        # ever take up more than 2 slots as there isn't a way for the
        # unittest to get those slots. This doesn't apply to threads, which
        # are passed through via MOOSE_PYTHONUNITTEST_NTHREADS below, so
        # min_threads/max_threads can differ like any other tester.
        min_value = params["min_parallel"]
        max_value = params["max_parallel"]

        # If the user sets min but not max, enforce max to be the min
        if min_value != 1 and max_value == 1:
            params["max_parallel"] = min_value
            self.addCaveats(f"max_parallel={min_value}")
        # Same for if they set the max but not the min
        elif max_value != 1 and min_value == 1:
            params["min_parallel"] = max_value
            self.addCaveats(f"min_parallel={max_value}")
        # Otherwise, just require that they are equal (if they set both)
        elif max_value != min_value:
            raise ValueError("min_parallel and max_parallel must be equal")

    def getCommand(self, options):
        """
        Returns the python command that executes unit tests
        """
        test_case = os.path.splitext(self.specs["input"])[0]
        if self.specs.isValid("test_case"):
            test_case += "." + self.specs["test_case"]

        use_buffer = " "
        if self.specs["buffer"]:
            use_buffer = " -b "

        if self.specs["separate"]:
            cmd = (
                os.path.join(
                    self.specs["moose_dir"], "scripts", "separate_unittests.py"
                )
                + " -f "
                + test_case
                + use_buffer
            )
        else:
            cmd = "python3 -m unittest" + use_buffer + "-v " + test_case

        return cmd + " ".join(self.specs["cli_args"])

    def checkRunnable(self, options):
        # Don't run unit tests on HPC. These tests commonly involve running
        # an appliacation within a black box script, which we cannot control
        # very well within the HPC environment
        if options.hpc:
            self.addCaveats("hpc unsupported")
            self.setStatus(self.skip)
            return False

        return super().checkRunnable(options)

    def getProcs(self, options):
        procs = super().getProcs(options)
        # If we start within a script within apptainer and then call mpiexec on HPC,
        # it will not work because the mpiexec call needs to be outside of the apptainer
        # call. So, limit these tests to 1 proc
        if (
            options.hpc
            and os.environ.get("APPTAINER_CONTAINER")
            and int(self.specs["min_parallel"]) == 1
            and procs != 1
        ):
            self.addCaveats("hpc apptainer max_cpus=1")
            return 1
        return procs

    def augmentEnvironment(self, options) -> dict:
        """
        Get environment variables that should be augmented while running.

        MOOSE_PYTHONUNITTEST_EXECUTABLE is added if an executable is
        available in the event that the unit test needs to use it. A
        common use case for this is MMS tests, which use a python
        unittest to run the moose executable.

        MOOSE_PYTHONUNITTEST_NTHREADS is added when this tester is
        requesting more than one thread (see getThreads), so that a unit
        test invoking the moose executable can pass it along via
        --n-threads. A common use case for this is MMS tests.
        """
        environment = super().augmentEnvironment(options)

        if options._app_name is not None:
            executable = self.specs["executable"]
            assert executable, "Executable is not set"
            environment["MOOSE_PYTHONUNITTEST_EXECUTABLE"] = executable

        nthreads = self.getThreads(options)
        if nthreads > 1:
            environment["MOOSE_PYTHONUNITTEST_NTHREADS"] = str(nthreads)

        return environment
