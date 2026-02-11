# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

import importlib.util
import inspect
import json
import os
import re
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Optional, Tuple

import mooseutils
from FactorySystem.InputParameters import InputParameters
from FactorySystem.MooseObject import MooseObject

from TestHarness import OutputInterface
from TestHarness.capability_util import CapabilityException, checkAppCapabilities
from TestHarness.StatusSystem import StatusSystem
from TestHarness.validation import ValidationCase, ValidationCaseClasses


class Tester(MooseObject, OutputInterface):
    """
    Base class from which all tester objects are instanced.
    """

    @staticmethod
    def validParams():
        params = MooseObject.validParams()

        # Common Options
        params.addRequiredParam(
            "type", "The type of test of Tester to create for this test."
        )
        params.addParam(
            "max_time",
            Tester.getDefaultMaxTime(),
            "The maximum in seconds that the test will be allowed to run.",
        )
        params.addParam("skip", "Provide a reason this test will be skipped.")
        params.addParam(
            "deleted",
            "Tests that only show up when using the '-e' option (Permanently skipped or not implemented).",
        )
        params.addParam("unique_test_id", "The unique hash given to a test")

        params.addParam(
            "heavy",
            False,
            "Set to True if this test should only be run when the '--heavy' option is used.",
        )
        params.addParam("group", [], "A list of groups for which this test belongs.")
        params.addParam(
            "prereq",
            [],
            "A list of prereq tests that need to run successfully before launching this test. When 'prereq = ALL', TestHarness will run this test last. Multiple 'prereq = ALL' tests, or tests that depend on a 'prereq = ALL' test will result in cyclic errors. Naming a test 'ALL' when using 'prereq = ALL' will also result in an error.",
        )
        params.addParam(
            "skip_checks",
            False,
            "Tells the TestHarness to skip additional checks (This parameter is set automatically by the TestHarness during recovery tests)",
        )
        params.addParam(
            "scale_refine", 0, "The number of refinements to do when scaling"
        )
        params.addParam("success_message", "OK", "The successful message")

        params.addParam(
            "cli_args", [], "Additional arguments to be passed to the test."
        )
        params.addParam(
            "use_shell",
            False,
            "Whether to use the shell as the executing program. This has the effect of prepending '/bin/sh -c ' to the command to be run.",
        )
        params.addParam(
            "allow_test_objects",
            False,
            "Allow the use of test objects by adding --allow-test-objects to the command line.",
        )

        params.addParam(
            "valgrind",
            "NONE",
            "Set to (NONE, NORMAL, HEAVY) to determine which configurations where valgrind will run.",
        )
        params.addParam(
            "max_buffer_size",
            None,
            "Bytes allowed in stdout/stderr before it is subjected to being trimmed. Set to -1 to ignore output size restrictions. "
            "If 'max_buffer_size' is not set, the default value of 'None' triggers a reasonable value (e.g. 100 kB)",
        )
        params.addParam(
            "parallel_scheduling",
            False,
            "Allow all tests in test spec file to run in parallel (adheres to prereq rules).",
        )

        # Test Filters
        params.addParam(
            "mesh_mode",
            ["ALL"],
            "A list of mesh modes for which this test will run ('DISTRIBUTED', 'REPLICATED')",
        )
        params.addParam(
            "recover", True, "A test that runs with '--recover' mode enabled"
        )
        params.addParam("restep", True, "A test that can run with --test-restep")
        params.addParam(
            "expect_exit_code", 0, "An integer exit code to expect from the test"
        )

        params.addParam(
            "capabilities",
            "",
            "A test that only runs if all listed capabilities are supported by the executable",
        )
        params.addParam(
            "dynamic_capabilities",
            False,
            "Whether or not to do a capability check that supports dynamic application loading",
        )
        params.addParam(
            "depend_files",
            [],
            "A test that only runs if all depend files exist (files listed are expected to be relative to the base directory, not the test directory",
        )
        params.addParam(
            "env_vars",
            [],
            "A test that only runs if all the environment variables listed are set",
        )
        params.addParam(
            "env_vars_not_set",
            [],
            "A test that only runs if all the environment variables listed are not set",
        )
        params.addParam(
            "should_execute",
            True,
            "Whether or not the executable needs to be run.  Use this to chain together multiple tests based off of one executeable invocation",
        )
        params.addParam(
            "required_submodule",
            [],
            "A list of initialized submodules for which this test requires.",
        )
        params.addParam("check_input", False, "Check for correct input file syntax")
        params.addParam(
            "display_required",
            False,
            "The test requires and active display for rendering (i.e., ImageDiff tests).",
        )
        params.addParam(
            "timing",
            True,
            "If True, the test will be allowed to run with the timing flag (i.e. Manually turning on performance logging).",
        )
        params.addParam(
            "python",
            None,
            "Restrict the test to s specific version of python (e.g., 3.6 or 3.7.1).",
        )
        params.addParam(
            "required_python_packages",
            None,
            "Test will only run if the supplied python packages exist.",
        )
        params.addParam(
            "requires",
            None,
            "A list of programs required for the test to operate, as tested with shutil.which.",
        )
        params.addParam(
            "working_directory",
            None,
            "When set, TestHarness will enter this directory before running test",
        )

        # SQA
        params.addParam(
            "requirement",
            None,
            "The SQA requirement that this test satisfies (e.g., 'The Marker system shall provide means to mark elements for refinement within a box region.')",
        )
        params.addParam(
            "design",
            [],
            "The list of markdown files that contain the design(s) associated with this test (e.g., '/Markers/index.md /BoxMarker.md').",
        )
        params.addParam(
            "issues",
            [],
            "The list of github issues associated with this test (e.g., '#1234 #4321')",
        )
        params.addParam(
            "detail", None, "Details of SQA requirement for use within sub-blocks."
        )
        params.addParam(
            "validation", False, "Set to True to mark test as a validation problem."
        )
        params.addParam(
            "verification", False, "Set to True to mark test as a verification problem."
        )
        params.addParam(
            "deprecated",
            False,
            "When True the test is no longer considered part SQA process and as such does not include the need for a requirement definition.",
        )
        params.addParam(
            "collections",
            [],
            "A means for defining a collection of tests for SQA process.",
        )
        params.addParam(
            "classification",
            "functional",
            "A means for defining a requirement classification for SQA process.",
        )

        # HPC
        params.addParam(
            "hpc_mem_per_cpu", "Memory requirement per CPU to use for HPC submission"
        )

        params.addParam(
            "validation_test", None, "List of validation scripts to run with this test"
        )

        # Add seach deprecated parameter with a useful docstring
        for param, capability in Tester.capability_params:
            help = "use capability '{capability}' " if capability else ""
            params.addParam(
                param,
                None,
                f"Deprecated parameter; {help} with 'capabilities' param instead",
            )

        return params

    capability_params: list[Tuple[str, Optional[str]]] = [
        ("boost", "boost"),
        ("chaco", "chaco"),
        ("compiler", "compiler"),
        ("dof_id_bytes", "dof_id_bytes"),
        ("exodus_version", "exodus"),
        ("fparser_jit", "fparser"),
        ("hpc", "hpc"),
        ("installation_type", "installation_type"),
        ("libpng", "libpng"),
        ("library_mode", "library_mode"),
        ("libtorch", "libtorch"),
        ("libtorch_version", "libtorch"),
        ("machine", "machine"),
        ("min_ad_size", "ad_size"),
        ("max_ad_size", "ad_size"),
        ("method", "method"),
        ("mumps", "mumps"),
        ("party", "party"),
        ("parmetis", "parmetis"),
        ("petsc_version", "petsc"),
        ("petsc_debug", "petsc_debug"),
        ("platform", "platform"),
        ("pthreads", "threads"),
        ("ptscotch", "ptscotch"),
        ("slepc", "slepc"),
        ("slepc_version", "slepc"),
        ("strumpack", "strumpack"),
        ("superlu", "superlu"),
        ("tecplot", "tecplot"),
        ("threading", None),
        ("vtk", "vtk"),
        ("vtk_version", "vtk"),
    ]
    """
    Parameters that are replaced with capabilities.

    This is a list of old param name -> capability name.
    """

    @staticmethod
    def augmentParams(params):
        # Augment our parameters with parameters from the validation test, if we
        # have any validation tests
        script = params["validation_test"]
        validation_classes = []
        if script:
            # Sanity checks
            if ".." in script:
                message = f"validation_test={script} out of test directory"
                raise ValueError(message)
            path = os.path.abspath(script)
            if not os.path.exists(script):
                message = f"validation_test={path} not found"
                raise FileNotFoundError(message)

            # Load the script; throw an exception here if it fails
            # so that the Parser can report a reasonable error
            spec = importlib.util.spec_from_file_location("validation_load", path)
            module = importlib.util.module_from_spec(spec)
            try:
                spec.loader.exec_module(module)
            except Exception as e:
                raise ImportError(f"In validation_test={path}:\n{e}")

            # Find the classes that are derived from the base validation
            # classes in the module (the user's python script)
            module_classes = inspect.getmembers(module, inspect.isclass)
            base_classes = [
                c[1] for c in module_classes if c[1] in ValidationCaseClasses
            ]
            other_classes = [c[1] for c in module_classes if c[1] not in base_classes]
            subclasses = [c for c in other_classes if issubclass(c, ValidationCase)]

            # Store each of the classes in the script that derives from
            # ValidationCase, and add their parameters to this Tester's
            # parameters
            validation_classes = []
            validation_params = InputParameters()
            for subclass in subclasses:
                validation_params = subclass.validParams()

                # Don't allow validation parameters that override
                # parameters from the Tester
                for key in validation_params.keys():
                    if key in params:
                        raise Exception(
                            f'Duplicate parameter "{key}" from validation test'
                        )

                # Collect the cumulative validation params
                validation_params += subclass.validParams()
                # Store the class so that it can be used later
                validation_classes.append(subclass)

            # Extend the Tester parameters
            params += validation_params

        # Store the classes that are used in validation so that they
        # can be constructed within the Job at a later time
        params.addPrivateParam("_validation_classes", validation_classes)

        return params

    # This is what will be checked for when we look for valid testers
    IS_TESTER = True

    @dataclass
    class JSONMetadata:
        path: os.PathLike
        data: Optional[dict] = None

    def __init__(self, name, params):
        MooseObject.__init__(self, name, params)
        OutputInterface.__init__(self)

        self.specs = params
        self.joined_out = ""
        self.process = None
        self.__caveats = set([])

        # Alternate text we want to print as part of our status instead of the
        # pre-formatted status text (SYNTAX PASS instead of OK for example)
        self.__tester_message = ""

        # Bool if test can run
        self._runnable = None

        # Set up common parameters
        self.should_execute = self.specs["should_execute"]
        self.check_input = self.specs["check_input"]

        if self.specs["allow_test_objects"]:
            self.specs["cli_args"].append("--allow-test-objects")

        # The Tester status; here we do not use locks because we need to
        # do deep copy operations of a Tester object, and thread locks
        # cannot be deep copied.
        self.test_status = StatusSystem(locking=False)
        # Enumerate the tester statuses we want to use
        self.no_status = self.test_status.no_status
        self.queued = self.test_status.queued
        self.skip = self.test_status.skip
        self.silent = self.test_status.silent
        self.success = self.test_status.success
        self.fail = self.test_status.fail
        self.diff = self.test_status.diff
        self.deleted = self.test_status.deleted
        self.error = self.test_status.error

        self.__failed_statuses = self.test_status.getFailingStatuses()
        self.__skipped_statuses = [self.skip, self.silent]

        # The command that we actually ended up running; this may change
        # depending on the runner which might inject something
        self.command_ran = None
        # The additional environment variables that we set when
        # running the test
        self.environment_ran: Optional[dict[str, str]] = None

        # Paths to additional JSON metadata that can be collected
        self.json_metadata: dict[str, Tester.JSONMetadata] = {}

        # The validation classes the user specified
        self._validation_classes = self.parameters()["_validation_classes"]

        # Check deprecated capability params
        bad_params = [
            (param, capability)
            for param, capability in self.capability_params
            if params[param] is not None
        ]
        if bad_params:
            entries = [
                (
                    f"'{param}'"
                    + (f" -> capability '{capability}'" if capability else "")
                )
                for param, capability in bad_params
            ]
            message = (
                "The following parameters are deprecated and should use the "
                "'capability' param instead; " + ", ".join(entries)
            )
            raise RuntimeError(message)

        self._augmented_capabilities: Optional[dict] = None

    def getStatus(self):
        return self.test_status.getStatus()

    def setStatus(self, status, message=""):
        self.__tester_message = message
        return self.test_status.setStatus(status)

    def createStatus(self):
        return self.test_status.createStatus()

    def getResultsEntry(self, options, create, graceful=False):
        """Get the entry in the results storage for this tester"""
        tests = options.results_storage["tests"]

        short_name = self.getTestNameShort()
        test_dir = self.getTestName()[: (-len(short_name) - 1)]
        test_dir_entry = tests.get(test_dir)
        if not test_dir_entry:
            if not create:
                if graceful:
                    return None, None
                raise Exception(f"Test folder {test_dir} not in results")
            tests[test_dir] = {"tests": {}}
            test_dir_entry = tests[test_dir]

        test_name_entry = test_dir_entry["tests"].get(short_name)
        test_dir_entry["spec_file"] = self.getSpecFile()
        if not test_name_entry:
            if not create:
                if graceful:
                    return test_dir_entry, None
                raise Exception(f"Test {test_dir}/{short_name} not in results")
            test_dir_entry["tests"][short_name] = {}
        return test_dir_entry, test_dir_entry["tests"][short_name]

    # Return a tuple (status, message, caveats) for this tester as found
    # in the previous results
    def previousTesterStatus(self, options):
        test_dir_entry, test_entry = self.getResultsEntry(options, False, True)
        status = (self.test_status.createStatus(), "", "")
        if test_entry:
            status_entry = test_entry["status"]
            status = (
                self.test_status.createStatus(str(status_entry["status"])),
                str(status_entry["status_message"]),
                status_entry["caveats"],
            )
        return status

    def getResults(self, options) -> dict:
        """Get the results dict for this Tester"""
        results = {
            "name": self.__class__.__name__,
            "command": self.getCommand(options),
            "input_file": self.getInputFile(),
        }
        json_metadata = {}
        for key, value in self.json_metadata.items():
            if value.data:
                json_metadata[key] = value.data
        if json_metadata:
            results["json_metadata"] = json_metadata
        return results

    def getStatusMessage(self):
        return self.__tester_message

    # Return a boolean based on current status
    def isNoStatus(self):
        return self.getStatus() == self.no_status

    def isSkip(self):
        return self.getStatus() in self.__skipped_statuses

    def isQueued(self):
        return self.getStatus() == self.queued

    def isSilent(self):
        return self.getStatus() == self.silent

    def isPass(self):
        return self.getStatus() == self.success

    def isFail(self):
        return self.getStatus() in self.__failed_statuses

    def isDiff(self):
        return self.getStatus() == self.diff

    def isDeleted(self):
        return self.getStatus() == self.deleted

    def isError(self):
        return self.getStatus() == self.error

    def getTestName(self):
        """return test name"""
        return self.specs["test_name"]

    def getTestNameShort(self):
        """return test short name (not including the path)"""
        return self.specs["test_name_short"]

    def getTestNameForFile(self):
        """return test short name for file creation ('/' to '.')"""
        return self.getTestNameShort().replace(os.sep, ".")

    def appendTestName(self, value):
        """
        Appends a value to the test name.

        Used when creating duplicate Testers for recover tests.
        """
        self.specs["test_name"] += value
        self.specs["test_name_short"] += value

    def getPrereqs(self):
        """return list of prerequisite tests this test depends on"""
        return self.specs["prereq"]

    def getMooseDir(self):
        """return moose directory"""
        return self.specs["moose_dir"]

    def getMoosePythonDir(self):
        """return moose directory"""
        return self.specs["moose_python_dir"]

    def getTestDir(self):
        """return directory in which this tester is located"""
        if self.specs["working_directory"]:
            return os.path.join(self.specs["test_dir"], self.specs["working_directory"])
        return self.specs["test_dir"]

    def getSpecFile(self):
        return os.path.join(self.specs["test_dir"], self.specs["spec_file"])

    def getMinReportTime(self):
        """return minimum time elapse before reporting a 'long running' status"""
        return self.specs["min_reported_time"]

    def getMaxTime(self):
        """return maximum time elapse before reporting a 'timeout' status"""
        return float(self.specs["max_time"])

    def setMaxTime(self, value):
        """
        Sets the max time for the job
        """
        self.specs["max_time"] = float(value)

    @staticmethod
    def getDefaultMaxTime():
        """
        Gets the default max run time
        """
        return int(os.getenv("MOOSE_TEST_MAX_TIME", 300))

    def getUniqueTestID(self):
        """return unique hash for test"""
        return (
            self.specs["unique_test_id"]
            if self.specs.isValid("unique_test_id")
            else None
        )

    def getRunnable(self, options):
        """return bool and cache results, if this test can run"""
        if self._runnable is None:
            self._runnable = self.checkRunnableBase(options)
        return self._runnable

    def getInputFile(self):
        """return the input file if applicable to this Tester"""
        return None

    def getInputFileContents(self):
        """return the contents of the input file applicable to this Tester"""
        return None

    def getOutputFiles(self, options):
        """return the output files if applicable to this Tester"""
        return [v.path for v in self.json_metadata.values()]

    def getCheckInput(self):
        return self.check_input

    def setValgrindMode(self, mode):
        """Increase the alloted time for tests when running with the valgrind option"""
        if mode == "NORMAL":
            self.setMaxTime(self.getMaxTime() * 2)
        elif mode == "HEAVY":
            self.setMaxTime(self.getMaxTime() * 6)

    def checkRunnable(self, options):
        """
        Derived method to return tuple if this tester should be executed or not.

        The tuple should be structured as (boolean, 'reason'). If false, and the
        reason is left blank, this tester will be treated as silent (no status
        will be printed and will not be counted among the skipped tests).
        """
        return True

    def shouldExecute(self):
        """
        return boolean for tester allowed to execute its command
        see .getCommand for more information
        """
        return self.should_execute

    def prepare(self, options):
        """
        Method which is called prior to running the test. It can be used to cleanup files
        or do other preparations before the tester is run.
        """
        return

    def getThreads(self, options):
        """return number of threads to use for this tester"""
        return 1

    def getProcs(self, options):
        """return number of processors to use for this tester"""
        return 1

    def getSlots(self, options):
        """return number of slots to use for this tester"""
        return self.getThreads(options) * self.getProcs(options)

    def hasOpenMPI(self):
        """return whether we have openmpi for execution

        The hacky way to do this is look for "ompi_info" (which only comes
        with openmpi), and then if it does exist make sure that "mpiexec" is
        in the same directory.

        We could probably move this somewhere so that it's not called multiple
        times, but I don't think that's a concern because the PATH should be
        very hot in cache and it's nice to keep this method local to where
        it's actually used.
        """
        which_ompi_info = shutil.which("ompi_info")
        if which_ompi_info is None:  # no ompi_info
            return False
        which_mpiexec = shutil.which("mpiexec")
        if which_mpiexec is None:  # no mpiexec
            return False
        return (
            Path(which_mpiexec).parent.absolute()
            == Path(which_ompi_info).parent.absolute()
        )

    def getCommand(self, options):
        """
        Return the command that the Tester wants ran

        We say "wants ran" here because the Runner may inject something
        within the command, for example when running within a container.
        Due to this distinction, you can obtain the command that was
        actually ran via getCommandRan()
        """
        return None

    def setCommandRan(self, command):
        """
        Sets the command that was actually ran.

        This is needed to account for running commands within containers
        and needing to run an additional command up front (i.e., with
        a pbs or slurm scheduler calling something like qsub)
        """
        self.command_ran = command

    def getCommandRan(self):
        """
        Gets the command that was actually ran.

        See setCommandRan() for the distinction.
        """
        return self.command_ran

    def setEnvironmentRan(self, value: dict):
        """Set the additional environment variables that the test was ran with."""
        assert isinstance(value, dict)
        self.environment_ran = value

    def getEnvironmentRan(self) -> Optional[dict[str, str]]:
        """Get the additional environment variables that the test was ran with."""
        return self.environment_ran

    def postSpawn(self, runner):
        """
        Entry point for after the process has been spawned
        """
        return

    def processResultsCommand(self, moose_dir, options):
        """method to return the commands (list) used for processing results"""
        return []

    def processResults(self, moose_dir, options, exit_code, runner_output):
        """method to process the results of a finished tester"""
        if self.specs["expect_exit_code"] != exit_code:
            reason = f'EXIT CODE {exit_code} != {self.specs["expect_exit_code"]}'
            self.setStatus(self.fail, str(reason))
            return f"\n\nExit Code: {exit_code}"

        return ""

    def hasRedirectedOutput(self, options):
        """return bool on tester having redirected output"""
        return (
            self.specs.isValid("redirect_output")
            and self.specs["redirect_output"] == True
            and self.getProcs(options) > 1
        )

    def getRedirectedOutputFiles(self, options):
        """return a list of redirected output"""
        if self.hasRedirectedOutput(options):
            return [
                os.path.join(self.getTestDir(), self.name() + ".processor.{}".format(p))
                for p in range(self.getProcs(options))
            ]
        return []

    def addCaveats(self, *kwargs):
        """Add caveat(s) which will be displayed with the final test status"""
        for i in [x for x in kwargs if x]:
            if type(i) == type([]):
                self.__caveats.update(i)
            else:
                self.__caveats.add(i)
        return self.getCaveats()

    def removeCaveat(self, caveat):
        """Removes a caveat, which _must_ exist"""
        self.__caveats.remove(caveat)

    def getCaveats(self):
        """Return caveats accumalted by this tester"""
        return self.__caveats

    def clearCaveats(self):
        """Clear any caveats stored in tester"""
        self.__caveats = set([])
        return self.getCaveats()

    def mustOutputExist(self, exit_code):
        """Whether or not we should check for the output once it has ran

        We need this because the PBS/slurm Runner objects, which use
        networked file IO, need to wait until the output is available on
        on the machine that submitted the jobs. A good example is RunException,
        where we should only look for output when we get a nonzero return
        code."""
        return exit_code == 0

    def getAugmentedCapabilities(self, options) -> dict:
        """Augment capabilities of the application with tester specs."""
        assert self.specs["capabilities"], "Shouldn't run without capabilities"
        return {}

    def checkCapabilities(self, options) -> Optional[str]:
        """
        Perform a check of the capabilities and required capabilities.

        Returns
        -------
        Optional[str]:
            A reason the check failed, if any.

        """
        if not self.specs["capabilities"]:
            # If this test has no capabilities, it will not depend
            # on anything in --only-tests-that-require
            if options._required_capabilities:
                return f"!{', !'.join(options._required_capabilities)}"
            return None

        # Add in augmented capabilities with options that are specific
        # to this test (mpi_procs, num_threads, etc)
        self._augmented_capabilities = self.getAugmentedCapabilities(options)

        # Try to check the capabilities; this could fail if the
        # capabilities string is bad or if the registry is bad
        try:
            present = checkAppCapabilities(
                options._capabilities,
                self.specs["capabilities"],
                bool(self.specs["dynamic_capabilities"]),
                add_capabilities=self._augmented_capabilities,
            )
        # Check failed, so add an error message to the Tester
        # that has a file:line link to the "capabilities" param
        except CapabilityException as e:
            self.setStatus(self.error, "INVALID CAPABILITIES")
            node = self.specs["_node"]
            output = ""
            if (filename := node.filename("capabilities")) or (
                filename := node.filename()
            ):
                output += f"{filename}:"
                if (line := node.line("capabilities")) or (line := node.line()):
                    output += f"{line}:\n"
                if fullpath := node.fullpath:
                    output += f"{fullpath[1:]}/capabilities:\n"
            output += f"{e}\n"
            self.appendOutput(output)
            return None

        # Capabilities are missing
        if not present:
            return f"Need {self.specs['capabilities']}"

        # Check required capabilities
        if options._required_capabilities:
            missing = []
            for value in options._required_capabilities:
                present = checkAppCapabilities(
                    options._capabilities,
                    self.specs["capabilities"],
                    True,
                    add_capabilities=self._augmented_capabilities,
                    negate_capabilities=[value],
                )
                if present:
                    missing.append(value)

            if missing:
                return f"!{', !'.join(missing)}"

        return None

    # need something that will tell  us if we should try to read the result
    def checkRunnableBase(self, options):
        """
        Method to check for caveats that would prevent this tester from
        executing correctly (or not at all).

        DO NOT override this method. Instead, see .checkRunnable()
        """
        reasons = {}
        checks = options._checks

        # If something has already deemed this test a failure
        if self.isFail():
            return False

        # Check if we only want to run syntax tests
        if options.check_input and not self.specs["check_input"]:
            self.setStatus(self.silent)
            return False

        # Check if we want to exclude syntax tests
        if options.no_check_input and self.specs["check_input"]:
            self.setStatus(self.silent)
            return False

        # Are we running only tests in a specific group?
        if options.group != "ALL" and options.group not in self.specs["group"]:
            self.setStatus(self.silent)
            return False
        if options.not_group != "" and options.not_group in self.specs["group"]:
            self.setStatus(self.silent)
            return False

        # Store regexp for matching tests if --re is used
        if options.reg_exp:
            match_regexp = re.compile(options.reg_exp)

        # If --re then only test matching regexp. Needs to run before other SKIP methods
        # This also needs to be in its own bucket group. We normally print skipped messages.
        # But we do not want to print tests that didn't match regex.
        if options.reg_exp and not match_regexp.search(self.specs["test_name"]):
            self.setStatus(self.silent)
            return False

        # Short circuit method and run this test if we are ignoring all caveats
        if options.ignored_caveats == "all":
            # Still, we should abide by the derived classes
            return self.checkRunnable(options)

        # Check for deleted tests
        if self.specs.isValid("deleted"):
            reasons["deleted"] = str(self.specs["deleted"])

        # Skipped by external means (example: TestHarness part2 with --check-input)
        if self.isSkip() and self.getStatusMessage():
            reasons["skip"] = self.getStatusMessage()
        # Test is skipped
        elif self.specs.type("skip") is bool and self.specs["skip"]:
            # Backwards compatible (no reason)
            reasons["skip"] = "no reason"
        elif self.specs.type("skip") is not bool and self.specs.isValid("skip"):
            reasons["skip"] = self.specs["skip"]
        # If were testing for SCALE_REFINE, then only run tests with a SCALE_REFINE set
        elif (options.scaling) and self.specs["scale_refine"] == 0:
            self.setStatus(self.silent)
            return False
        # If we're testing with valgrind, then skip tests that require parallel or threads or don't meet the valgrind setting
        elif options.valgrind_mode != "":
            tmp_reason = ""
            if self.specs["valgrind"].upper() != options.valgrind_mode.upper():
                tmp_reason = "Valgrind==" + self.specs["valgrind"].upper()
            elif int(self.specs["min_threads"]) > 1:
                tmp_reason = "Valgrind requires non-threaded"
            elif self.specs["check_input"]:
                tmp_reason = "check_input==True"
            if tmp_reason != "":
                reasons["valgrind"] = tmp_reason
        # If we're running in recover mode skip tests that have recover = false
        elif options.enable_recover and self.specs["recover"] == False:
            reasons["recover"] = "NO RECOVER"
        elif options.enable_restep and self.specs["restep"] == False:
            reasons["restep"] = "NO RESTEP"

        # Check for supported capabilities
        if capabilities_reason := self.checkCapabilities(options):
            reasons["capabilities"] = capabilities_reason

        # PETSc and SLEPc is being explicitly checked above
        local_checks = [
            "mesh_mode",
        ]

        for check in local_checks:
            test_platforms = set()
            operator_display = "!="
            inverse_set = False
            for x in self.specs[check]:
                if x[0] == "!":
                    if inverse_set:
                        reasons[check] = "Multiple Negation Unsupported"
                    inverse_set = True
                    operator_display = "=="
                    x = x[1:]  # Strip off the !
                x_upper = x.upper()
                if x_upper in test_platforms:
                    reasons[x_upper] = "Duplicate Entry or Negative of Existing Entry"
                test_platforms.add(x.upper())

            match_found = len(test_platforms.intersection(checks[check])) > 0
            # Either we didn't find the match when we were using normal "include" logic
            # or we did find the match when we wanted to exclude it
            if inverse_set == match_found:
                reasons[check] = (
                    re.sub(r"\[|\]", "", check).upper()
                    + operator_display
                    + ", ".join(test_platforms)
                )

        # Check for heavy tests
        if options.all_tests or options.heavy_tests:
            if not self.specs["heavy"] and options.heavy_tests:
                reasons["heavy"] = "NOT HEAVY"
        elif self.specs["heavy"]:
            reasons["heavy"] = "HEAVY"

        # Check to make sure depend files exist
        for file in self.specs["depend_files"]:
            if not os.path.isfile(os.path.join(self.specs["base_dir"], file)):
                reasons["depend_files"] = "DEPEND FILES"

        # Check to make sure required submodules are initialized
        for var in self.specs["required_submodule"]:
            if var not in checks["submodules"]:
                reasons["required_submodule"] = "%s submodule not initialized" % var

        # Check to make sure environment variable exists
        for var in self.specs["env_vars"]:
            if not os.environ.get(var):
                reasons["env_vars"] = "ENV VAR NOT SET"

        for var in self.specs["env_vars_not_set"]:
            if os.environ.get(var):
                reasons["env_vars"] = "ENV VAR SET"

        # Check for display
        if self.specs["display_required"] and not os.getenv("DISPLAY", False):
            reasons["display_required"] = "NO DISPLAY"

        # Check python version
        py_version = self.specs["python"]
        if py_version is not None:
            if isinstance(py_version, int) and (py_version != sys.version_info[0]):
                reasons["python"] = "PYTHON != {}".format(py_version)
            elif isinstance(py_version, float) and (
                py_version != float("{}.{}".format(*sys.version_info[0:2]))
            ):
                reasons["python"] = "PYTHON != {}".format(py_version)
            elif isinstance(py_version, str):
                ver = py_version.split(".")
                if any(sys.version_info[i] != int(v) for i, v in enumerate(ver)):
                    reasons["python"] = "PYTHON != {}".format(py_version)

        # Check python packages
        py_packages = self.specs["required_python_packages"]
        if py_packages is not None:
            missing = mooseutils.check_configuration(py_packages.split(), message=False)
            if missing:
                reasons["python_packages_required"] = ", ".join(
                    ["{}".format(p) for p in missing]
                )

        # Check for programs
        programs = self.specs["requires"]
        if programs is not None:
            missing = []
            for prog in programs.split():
                if shutil.which(prog) is None:
                    missing.append(prog)
            if missing:
                reasons["requires"] = ", ".join(["no {}".format(p) for p in missing])

        # Verify working_directory is relative. We'll check to make sure it's available just in time.
        if self.specs["working_directory"]:
            if self.specs["working_directory"][:1] == os.path.sep:
                self.setStatus(self.fail, "ABSOLUTE PATH DETECTED")
            # We can't offer the option of reading output files outside of initial TestDir
            if ".." in self.specs["working_directory"] and options.sep_files:
                reasons["working_directory"] = "--sep-files enabled"

        # Use shell not supported for HPC
        if self.specs["use_shell"] and options.hpc:
            reasons["use_shell"] = "no use_shell with hpc"

        ##### The below must be performed last to register all above caveats #####
        # Remove any matching user supplied caveats from accumulated checkRunnable caveats that
        # would normally produce a skipped test.
        caveat_list = set()
        if options.ignored_caveats:
            caveat_list = set([x.lower() for x in options.ignored_caveats.split()])

        if len(set(reasons.keys()) - caveat_list) > 0:
            tmp_reason = []
            for key, value in reasons.items():
                if key.lower() not in caveat_list:
                    tmp_reason.append(value)

            flat_reason = ", ".join(tmp_reason)
            self.addCaveats(flat_reason)

            # Reasons we wish to silence tests
            if "deleted" in reasons.keys():
                self.setStatus(self.silent)
            elif (
                "heavy" in reasons.keys()
                and options.heavy_tests
                and not self.specs["heavy"]
            ):
                self.setStatus(self.silent)

            # Failed already (cannot run)
            elif self.getStatus() == self.fail:
                return False
            else:
                self.setStatus(self.skip)
            return False

        # Check the return values of the derived classes
        self._runnable = self.checkRunnable(options)
        return self._runnable

    def needFullOutput(self, options):
        """
        Whether or not the full output is needed.

        If this is True, it means that we cannot truncate
        the stderr/stdout output. This is often needed
        when we're trying to read something from the output.
        """
        return False

    def run(self, options, exit_code, runner_output):
        output = self.processResults(
            self.getMooseDir(), options, exit_code, runner_output
        )
        if output:
            output = output.rstrip() + "\n\n"

        # Load metadata if it exists
        if not self.isSkip() and self.json_metadata:
            output += "Loading JSON metadata...\n"
            if exit_code == 0:
                for key, entry in self.json_metadata.items():
                    path = os.path.join(self.getTestDir(), entry.path)
                    prefix = f"  {key} ({path}): "
                    if os.path.isfile(path):
                        try:
                            with open(path, "r") as f:
                                entry.data = json.load(f)
                        except:
                            output += f"{prefix}cannot be loaded\n"
                            self.setStatus(self.fail, "BAD METADATA")
                        else:
                            output += f"{prefix}loaded\n"
                    else:
                        output += f"{prefix}does not exist\n"
                        self.setStatus(self.fail, "MISSING METADATA")
            else:
                output += "  Not loading due to non-zero exit code\n"
            output += "\n"

        # If the tester requested to be skipped at the last minute, report that.
        if self.isSkip():
            output += f"\nTester skipped, reason: {self.getStatusMessage()}\n"
        elif self.isFail():
            output += f"\nTester failed, reason: {self.getStatusMessage()}\n"

        self.setOutput(output)

    def getHPCPlace(self, options):
        """
        Return the placement to use for HPC jobs
        """
        if options.hpc_scatter_procs:
            procs = self.getProcs(options)
            if procs > 1 and procs <= options.hpc_scatter_procs:
                return "scatter"
        return "free"

    def augmentEnvironment(self, options) -> dict:
        """Get environment variables that should be augmented while running."""
        # Explicitly set OMP_NUM_THREADS to the number of threads
        return {"OMP_NUM_THREADS": f"{self.getThreads(options)}"}

    def getOutputPathPrefix(self, options) -> str:
        """
        Returns a file prefix that is unique to this test

        Should be used for all TestHarness produced files for this test
        """
        return os.path.join(self.getOutputDirectory(options), self.getTestNameForFile())

    def getOutputDirectory(self, options):
        """Get the directory for output for this job"""
        if not options.output_dir:
            return self.getTestDir()
        return os.path.join(
            options.output_dir,
            self.getTestName()[: -len(self.getTestNameShort()) - 1],
        )
