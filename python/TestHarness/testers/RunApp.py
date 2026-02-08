#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re, os, shutil
from Tester import Tester
from TestHarness import util, TestHarness
from TestHarness.capability_util import addAugmentedCapability
from shlex import quote
import json
from typing import Optional

class RunApp(Tester):

    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addParam('input',              "The input file to use for this test.")
        params.addParam('test_name',          "The name of the test - populated automatically")
        params.addParam('input_switch', '-i', "The default switch used for indicating an input to the executable")
        params.addParam('errors',             ['ERROR', 'command not found', 'terminate called after throwing an instance of'], "The error messages to detect a failed run")
        params.addParam('expect_out',         "A regular expression or literal string that must occur in the output in order for the test to be considered passing (see match_literal).")
        params.addParam('match_literal', False, "Treat expect_out as a string not a regular expression.")
        params.addParam('absent_out',         "A regular expression that must be *absent* from the output for the test to pass.")
        params.addParam('executable_pattern', "A test that only runs if the executable name matches the given pattern")
        params.addParam('delete_output_before_running',  True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")
        params.addParam('custom_evaluation_script', False, "A .py file containing a custom function for evaluating a test's success. For syntax, please check https://mooseframework.inl.gov/python/TestHarness.html")

        # RunApp can also run arbitrary commands. If the "command" parameter is supplied
        # it'll be used in lieu of building up the command automatically
        params.addParam('command',       "The command line to execute for this test")
        params.addParam('command_proxy', "A proxy command to run that will execute the underlying test via wrapper; the intended command is set in the env variable RUNAPP_COMMAND")

        # Parallel/Thread testing
        params.addParam('max_parallel', 1000, "Maximum number of MPI processes this test can be run with      (Default: 1000)")
        params.addParam('min_parallel',    1, "Minimum number of MPI processes that this test can be run with (Default: 1)")
        params.addParam('max_threads',    16, "Max number of threads (Default: 16)")
        params.addParam('min_threads',     1, "Min number of threads (Default: 1)")
        params.addParam('redirect_output',  False, "Redirect stdout to files. Necessary when expecting an error when using parallel options")

        params.addParam('allow_warnings',   True, "Whether or not warnings are allowed.  If this is False then a warning will be treated as an error.  Can be globally overridden by setting 'allow_warnings = False' in the testroot file.");
        params.addParam('allow_unused',   False, "Whether or not unused parameters are allowed in the input file.  Can be globally overridden by setting 'allow_unused = False' in the testroot file.");
        params.addParam('allow_override', True, "Whether or not overriding a parameter/block in the input file generates an error.  Can be globally overridden by setting 'allow_override = False' in the testroot file.");
        params.addParam('allow_deprecated', True, "Whether or not deprecated warnings are allowed.  Setting to False will cause deprecation warnings to be treated as test failures.  We do NOT recommend you globally set this permanently to False!  Deprecations are a part of the normal development flow and _SHOULD_ be allowed!")
        params.addParam('no_error_deprecated', False, "Don't pass --error-deprecated on the command line even when running the TestHarness with --error-deprecated")
        params.addParam('no_additional_cli_args', False, "A Boolean indicating that no additional CLI args should be added from the TestHarness. Note: This parameter should be rarely used as it will not pass on additional options such as those related to mpi, threads, distributed mesh, errors, etc.")

        params.addParam('capture_perf_graph', True, 'Whether or not to enable the capturing of PerfGraph output via Outputs/perf_graph_json_file and --capture-perf-graph')
        params.addParam("perf_graph_live", False, "Whether to enable perf graph live printing")

        # Valgrind
        params.addParam('valgrind', 'NORMAL', "Set to (NONE, NORMAL, HEAVY) to determine which configurations where valgrind will run.")

        device_list_str = "', '".join(d.upper() for d in TestHarness.validComputeDevices())
        device_param_doc = f"The devices to use for this libtorch or MFEM test ('{device_list_str}'); device availability depends on library support and compilation settings; default ('CPU')"
        params.addParam('compute_devices', ['CPU'], device_param_doc)

        return params

    def __init__(self, name, params):
        Tester.__init__(self, name, params)
        if os.environ.get("MOOSE_MPI_COMMAND"):
            self.mpi_command = os.environ['MOOSE_MPI_COMMAND']
            self.force_mpi = True
        else:
            self.mpi_command = 'mpiexec'
            self.force_mpi = False

        # Make sure that either input or command is supplied
        if not (params.isValid('input') or params.isValid('command') or params.isValid("command_proxy") or params['no_additional_cli_args']):
            raise Exception('One of "input", "command", "command_proxy", or "no_additional_cli_args" must be supplied for a RunApp test')

        if params.isValid('command_proxy'):
            params['use_shell'] = True
            # Not compatible with each other due to the return break in runCommand()
            if params['no_additional_cli_args']:
                raise Exception('The parameters "command_proxy" and "no_additional_cli_args" cannot be supplied together')

        for value in params['compute_devices']:
            if value.lower() not in TestHarness.validComputeDevices():
                raise Exception(f'Unknown device "{value}"')

        # The capabilities file that we need to set with
        # --testharness-capabilities, if any. This should
        # only be valid if we have a 'capabilities' spec
        # and any of those capabilities depend on the
        # augmented capabilities
        self._augmented_capabilities_file: Optional[str] = None

    def getInputFile(self):
        if self.specs.isValid('input'):
            return os.path.join(self.getTestDir(), self.specs['input'].strip())
        else:
            return None # Not all testers that inherit from RunApp have an input file

    def getInputFileContents(self):
        input_file = self.getInputFile()
        if input_file and os.path.exists(os.path.join(self.getTestDir(), input_file)):
            with open(os.path.join(self.getTestDir(), input_file), 'r') as f:
                input_file = f.read()
        # Test is supposed to have an input file, but we couldn't find it. Don't error on that event here, and
        # instead allow the TestHarness to continue. It will error appropriately elsewhere.
        else:
            input_file = None
        return input_file

    def checkRunnable(self, options):
        if options.enable_recover or options.enable_restep:
            reason = 'RECOVER' if options.enable_recover else 'RESTEP'
            caveats = []
            for param in ['expect_out', 'absent_out']:
                if self.specs.isValid(param):
                    caveats.append(param)
            if self.specs["expect_exit_code"] != 0:
                caveats.append("nonzero_exit")
            if caveats:
                self.addCaveats(f'{",".join(caveats)} {reason}')
                self.setStatus(self.skip)
                return False

        if self.specs.isValid('executable_pattern') and re.search(self.specs['executable_pattern'], self.specs['executable']) == None:
            self.addCaveats('EXECUTABLE PATTERN')
            self.setStatus(self.skip)
            return False

        devices_lower = [x.lower() for x in self.specs['compute_devices']]
        if options.compute_device not in devices_lower:
            self.addCaveats(f'{options.compute_device} not in compute devices')
            self.setStatus(self.skip)
            return False

        if options.hpc and self.specs.isValid('command_proxy') and os.environ.get('APPTAINER_CONTAINER') is not None:
            self.addCaveats('hpc unsupported')
            self.setStatus(self.skip)
            return False

        # Finalizing output using the current method in the submission script from the rank 0 process isn't
        # really a good idea when output might exist on a different node. We could make that finalization
        # more complex, but there isn't a need at the moment.
        if options.hpc and self.specs['redirect_output'] == True and int(self.specs['min_parallel']) > 1:
            self.addCaveats('hpc min_cpus=1')
            self.setStatus(self.skip)
            return False

        # Setup the capturing of perf graph data, if enabled and not in a case
        # where it doesn't make sense to do it
        if options.capture_perf_graph:
            assert 'perf_graph' not in self.json_metadata
            skip = (
                not self.specs["capture_perf_graph"]
                or self.specs["expect_exit_code"] != 0
                or self.specs["no_additional_cli_args"]
                or self.getCheckInput()
                or "--check-input" in self.specs["cli_args"]
                or "--mesh-only" in self.specs["cli_args"]
                or "--split-mesh" in self.specs["cli_args"]
                or (self.specs.isValid("input") and not self.specs["input"])
                or not self.specs["should_execute"]
            )
            if skip:
                self.addCaveats('no --capture-perf-graph')
            else:
                file = 'metadata_perf_graph_' + self.getTestNameForFile() + '.json'
                self.json_metadata['perf_graph'] = Tester.JSONMetadata(file)

        return True

    def getThreads(self, options):
        # This disables additional arguments
        if self.specs['no_additional_cli_args']:
            return 1

        # Set number of threads to be used lower bound
        nthreads = max(options.nthreads, int(self.specs['min_threads']))
        # Set number of threads to be used upper bound
        nthreads = min(nthreads, int(self.specs['max_threads']))

        if nthreads > options.nthreads:
            self.addCaveats('min_threads=' + str(nthreads))
        elif nthreads < options.nthreads:
            self.addCaveats('max_threads=' + str(nthreads))

        return nthreads

    def getProcs(self, options):
        # This disables additional arguments
        if self.specs['no_additional_cli_args']:
            return 1

        if options.parallel == None:
            default_ncpus = 1
        else:
            default_ncpus = options.parallel

        min_parallel = int(self.specs['min_parallel'])

        # Raise the floor
        ncpus = max(default_ncpus, min_parallel)
        # Lower the ceiling
        ncpus = min(ncpus, int(self.specs['max_parallel']))

        # Finalizing output using the current method in the submission script from the rank 0 process isn't
        # really a good idea when output might exist on a different node. We could make that finalization
        # more complex, but there isn't a need at the moment.
        if options.hpc and self.specs['redirect_output'] == True and min_parallel == 1 and ncpus > 1:
            self.addCaveats('hpc min_cpus=1')
            return 1

        if ncpus > default_ncpus:
            self.addCaveats('min_cpus=' + str(ncpus))
        elif ncpus < default_ncpus:
            self.addCaveats('max_cpus=' + str(ncpus))

        return ncpus

    def getCommand(self, options):
        specs = self.specs

        # Just return an arbitrary command if one is supplied
        if specs.isValid('command'):
            return os.path.join(specs['test_dir'], specs['command']) + ' ' + ' '.join(specs['cli_args'])

        # Check for built application
        if shutil.which(specs['executable']) is None:
            self.setStatus(self.error, 'APPLICATION NOT FOUND')

        # If no_additional_cli_args is set to True, return early with a simplified command line ignoring
        # all other TestHarness supplied options.
        if specs['no_additional_cli_args']:
            # TODO: Do error checking for TestHarness options that will be silently ignored
            cmd = os.path.join(specs['test_dir'], specs['executable']) + ' ' + ' '.join(specs['cli_args'])

            # Need to run mpiexec with containerized openmpi
            if options.hpc and self.hasOpenMPI():
                cmd = f'mpiexec -n 1 {cmd}'

            return cmd

        # Create the additional command line arguments list
        cli_args = list(specs['cli_args'])

        if specs["capabilities"]:
            # Check that the app supports the capabilities we think it does
            cli_args.append(f"--required-capabilities={quote(specs['capabilities'])}")

            # If we have augmented capabilities (capabilities not in the app)
            # that we checked against, they need to be augmented in the app
            if self._augmented_capabilities_file is not None:
                cli_args.append(
                    f"--testharness-capabilities={quote(self._augmented_capabilities_file)}"
                )
        else:
            assert self._augmented_capabilities_file is None

        if options.distributed_mesh and '--distributed-mesh' not in cli_args:
            # The user has passed the parallel-mesh option to the test harness
            # and it is NOT supplied already in the cli-args option
            cli_args.append('--distributed-mesh')

        if specs['restep'] != False and options.enable_restep:
            cli_args.append('--test-restep')

        if not specs['perf_graph_live'] and '--disable-perf-graph-live' not in cli_args:
            cli_args.append('--disable-perf-graph-live')

        if '--error' not in cli_args and (not specs["allow_warnings"] or options.error) and not options.allow_warnings:
            cli_args.append('--error')

        if '--error-unused' not in cli_args and options.error_unused:
            cli_args.append('--error-unused')

        if '--error-unused' not in cli_args and '--allow-unused' not in cli_args and (specs["allow_unused"] or options.allow_unused):
            cli_args.append('--allow-unused')

        if '--error-override' not in cli_args and not specs["allow_override"]:
            cli_args.append('--error-override')

        if '--error-deprecated' not in cli_args and not specs["no_error_deprecated"] and (not specs["allow_deprecated"] or options.error_deprecated):
            cli_args.append('--error-deprecated')

        if self.getCheckInput():
            cli_args.append('--check-input')

        if options.timing and specs["timing"]:
            cli_args.append('--timing')
            cli_args.append('Outputs/perf_graph=true')

        pg_metadata = self.json_metadata.get('perf_graph')
        if pg_metadata:
            path = os.path.join(self.getTestDir(), pg_metadata.path)
            cli_args.append(f'Outputs/perf_graph_json_file={path}')

        if options.colored == False:
            cli_args.append('--color off')

        if options.cli_args:
            cli_args.insert(0, options.cli_args)

        if options.scaling and specs['scale_refine'] > 0:
            cli_args.insert(0, ' -r ' + str(specs['scale_refine']))

        # Get the number of processors and threads the Tester requires
        ncpus = self.getProcs(options)
        nthreads = self.getThreads(options)
        cli_args.append(f'--compute-device={options.compute_device}')

        if specs['redirect_output'] and ncpus > 1:
            cli_args.append('--keep-cout --redirect-output ' + self.name())

        # Append additional global command line arguments
        if options.append_runapp_cliarg:
            append_cliarg = options.append_runapp_cliarg
            if isinstance(append_cliarg, str):
                append_cliarg = [append_cliarg]
            cli_args += append_cliarg

        command = specs['executable'] + ' '
        if len(specs['input']) > 0: # only apply if we have input set
            command += specs['input_switch'] + ' ' + specs['input'] + ' '
        command += ' '.join(cli_args)
        if options.valgrind_mode.upper() == specs['valgrind'].upper() or options.valgrind_mode.upper() == 'HEAVY' and specs['valgrind'].upper() == 'NORMAL':
            command = 'valgrind --suppressions=' + os.path.join(specs['moose_dir'], 'python', 'TestHarness', 'suppressions', 'errors.supp') + ' --leak-check=full --tool=memcheck --dsymutil=yes --track-origins=yes --demangle=yes --enable-debuginfod=no -v ' + command
        elif nthreads > 1:
            command = command + ' --n-threads=' + str(nthreads)

        # Force mpi, more than 1 core, or containerized openmpi (requires mpiexec serial)
        if self.force_mpi or ncpus > 1 or (options.hpc and self.hasOpenMPI()):
            command = f'{self.mpi_command} -n {ncpus} {command}'

        # Arbitrary proxy command, but keep track of the command so that someone could use it later
        if specs.isValid('command_proxy'):
            command = command.replace('"', r'\"')
            return f'RUNAPP_COMMAND="{command}" {os.path.join(specs["test_dir"], specs["command_proxy"])}'

        return command

    def testFileOutput(self, moose_dir, options, runner_output):
        """ Set a failure status for expressions found in output """
        reason = ''
        errors = ''
        specs = self.specs

        if specs["custom_evaluation_script"]:
            if (specs.isValid('expect_out') or specs.isValid('absent_out')):
                errors += 'expect_out and absent_out can not be supplied when using a custom evaluation function!'
                self.setStatus(self.fail, "CUSTOM EVAL FAILED")
                return errors
            import importlib.util, sys
            custom_mod_spec = importlib.util.spec_from_file_location("custom_module", os.path.join(self.getMooseDir(),self.getTestDir(), specs["custom_evaluation_script"]))
            custom_module = importlib.util.module_from_spec(custom_mod_spec)
            sys.modules['custom_module'] = custom_module
            custom_mod_spec.loader.exec_module(custom_module)
            if custom_module.custom_evaluation(runner_output):
                return errors
            else:
                errors += util.outputHeader('Custom evaluation failed', ending=False)
                self.setStatus(self.fail, "CUSTOM EVAL FAILED")
                return errors

        params_and_msgs = {'expect_err':
                              {'error_missing': True,
                               'modes': ['ALL'],
                               'reason': "EXPECTED ERROR MISSING",
                               'message': "Unable to match the following {} against the program's output:"},
                           'expect_assert':
                              {'error_missing': True,
                               'modes': ['dbg', 'devel'],
                               'reason': "EXPECTED ASSERT MISSING",
                               'message': "Unable to match the following {} against the program's output:"},
                           'expect_out':
                               {'error_missing': True,
                                'modes': ['ALL'],
                                'reason': "EXPECTED OUTPUT MISSING",
                                'message': "Unable to match the following {} against the program's output:"},
                           'absent_out':
                               {'error_missing': False,
                                'modes': ['ALL'],
                                'reason': "OUTPUT NOT ABSENT",
                                'message': "Matched the following {}, which we did NOT expect:"}
                           }

        for param,attr in params_and_msgs.items():
            if specs.isValid(param) and (options.method in attr['modes'] or attr['modes'] == ['ALL']):
                match_type = ""
                if specs['match_literal']:
                    have_expected_out = util.checkOutputForLiteral(runner_output, specs[param])
                    match_type = 'literal'
                else:
                    have_expected_out = util.checkOutputForPattern(runner_output, specs[param])
                    match_type = 'pattern'

                # Exclusive OR test
                if attr['error_missing'] ^ have_expected_out:
                    reason = attr['reason']
                    errors += util.outputHeader(attr['message'].format(match_type) + "\n\n" + specs[param],
                                                ending=False)
                    break

        if reason != '':
            self.setStatus(self.fail, reason)

        return errors

    def testExitCodes(self, options, exit_code, runner_output):
        specs = self.specs
        reason = None

        if specs["expect_exit_code"] != exit_code:
            reason = f'EXIT CODE {exit_code} != {specs["expect_exit_code"]}'
        elif (
            options.valgrind_mode == ""
            and not specs.isValid("expect_err")
            and len([x for x in filter(lambda x: x in runner_output, specs["errors"])])
            > 0
        ):
            reason = "ERRMSG"
        # Valgrind runs
        elif (
            exit_code == 0
            and self.shouldExecute()
            and options.valgrind_mode != ""
            and "ERROR SUMMARY: 0 errors" not in runner_output
        ):
            reason = "MEMORY ERROR"

        if reason:
            self.setStatus(self.fail, str(reason))
            return "\n\nExit Code: " + str(exit_code)

        return ""

    def processResults(self, moose_dir, options, exit_code, runner_output):
        """
        Wrapper method for testFileOutput.

        testFileOutput does not set a success status, while processResults does.
        For testers that are RunApp types, they will call this method (processResults).

        Other tester types (like exodiff) will call testFileOutput. This is to prevent
        derived testers from having a successful status set, before actually running
        the derived processResults method.

        # TODO: because RunParallel is now setting every successful status message,
                refactor testFileOutput and processResults.
        """
        # If we had capability requirements and get an exit 77, it means that the
        # capability doesn't exist in the binary
        if self.specs["capabilities"] and exit_code == 77:
            self.setStatus(self.skip, "CAPABILITIES")
            self.addCaveats(self.specs["capabilities"])
            return ""

        # Parent check; check exit code
        if parent_out := super().processResults(
            moose_dir, options, exit_code, runner_output
        ):
            return parent_out

        output = ''
        output += self.testFileOutput(moose_dir, options, runner_output)
        if self.isNoStatus():
            output += self.testExitCodes(options, exit_code, runner_output)

        return output

    def mustOutputExist(self, exit_code):
        if self.specs["expect_exit_code"] != 0:
            return exit_code != 0
        return exit_code == 0

    def needFullOutput(self, options):
        # We need the full output when we're trying to read from said output
        params = ['expect_err', 'expect_assert', 'expect_out', 'absent_out']
        for param in params:
            if self.specs.isValid(param):
                return True
        return super().needFullOutput(options)

    def getAugmentedCapabilities(self, options) -> dict:
        augmented_capabilities = super().getAugmentedCapabilities(options)

        def augment_capability(*args, **kwargs):
            addAugmentedCapability(
                options._capabilities, augmented_capabilities, *args, **kwargs
            )

        # NOTE: If you add to this list, it must be added to
        # CapabilityRegistry::augmented_capability_names in
        # framework/src/base/CapabilityRegistry.C
        augment_capability(
            "mpi_procs",
            self.getProcs(options),
            "Number of MPI processes",
            explicit=True,
        )
        augment_capability(
            "num_threads",
            self.getThreads(options),
            "Number of threads",
            explicit=True,
        )

        return augmented_capabilities

    def getCapabilitiesFilePath(self, options) -> str:
        return f"{self.getOutputPathPrefix(options)}_testharness_capabilities.json"

    def prepare(self, options):
        super().prepare(options)

        # Dump the augmented capabilities, if any
        if self.specs["capabilities"]:
            # Capabilities from this Tester's specs in addition
            # to capabilities from the global options
            capabilities = (
                self._augmented_capabilities | options._augmented_capabilities
            )

            # Capture the capabilities that we need to dump, if any.
            # For now, we'll lazily just see if each of the
            # augmented capabilities exists in the whole string.
            store_capabilities = {}
            for capability, entry in capabilities.items():
                if capability in self.specs["capabilities"]:
                    store_capabilities[capability] = entry

            # We have capabilities to store
            if store_capabilities:
                self._augmented_capabilities_file = self.getCapabilitiesFilePath(
                    options
                )
                with open(self._augmented_capabilities_file, "w") as f:
                    json.dump(store_capabilities, f)
