#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import re, os, shutil
from Tester import Tester
from TestHarness import util

class RunApp(Tester):

    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addParam('input',              "The input file to use for this test.")
        params.addParam('test_name',          "The name of the test - populated automatically")
        params.addParam('input_switch', '-i', "The default switch used for indicating an input to the executable")
        params.addParam('errors',             ['ERROR', 'command not found', 'terminate called after throwing an instance of'], "The error messages to detect a failed run")
        params.addParam('expect_out',         "A regular expression or literal string that must occur in the input in order for the test to be considered passing (see match_literal).")
        params.addParam('match_literal', False, "Treat expect_out as a string not a regular expression.")
        params.addParam('absent_out',         "A regular expression that must be *absent* from the output for the test to pass.")
        params.addParam('should_crash', False, "Inidicates that the test is expected to crash or otherwise terminate early")
        params.addParam('executable_pattern', "A test that only runs if the executable name matches the given pattern")
        params.addParam('delete_output_before_running',  True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")
        params.addParam('delete_output_folders', True, "Delete output folders before running")
        params.addParam('custom_evaluation_script', False, "A .py file containing a custom function for evaluating a test's success. For syntax, please check https://mooseframework.inl.gov/python/TestHarness.html")

        # RunApp can also run arbitrary commands. If the "command" parameter is supplied
        # it'll be used in lieu of building up the command automatically
        params.addParam('command',            "The command line to execute for this test.")

        # Parallel/Thread testing
        params.addParam('max_parallel', 1000, "Maximum number of MPI processes this test can be run with      (Default: 1000)")
        params.addParam('min_parallel',    1, "Minimum number of MPI processes that this test can be run with (Default: 1)")
        params.addParam('max_threads',    16, "Max number of threads (Default: 16)")
        params.addParam('min_threads',     1, "Min number of threads (Default: 1)")
        params.addParam('redirect_output',  False, "Redirect stdout to files. Neccessary when expecting an error when using parallel options")

        params.addParam('allow_warnings',   True, "Whether or not warnings are allowed.  If this is False then a warning will be treated as an error.  Can be globally overridden by setting 'allow_warnings = False' in the testroot file.");
        params.addParam('allow_unused',   False, "Whether or not unused parameters are allowed in the input file.  Can be globally overridden by setting 'allow_unused = False' in the testroot file.");
        params.addParam('allow_override', True, "Whether or not overriding a parameter/block in the input file generates an error.  Can be globally overridden by setting 'allow_override = False' in the testroot file.");
        params.addParam('allow_deprecated', True, "Whether or not deprecated warnings are allowed.  Setting to False will cause deprecation warnings to be treated as test failures.  We do NOT recommend you globally set this permanently to False!  Deprecations are a part of the normal development flow and _SHOULD_ be allowed!")
        params.addParam('no_error_deprecated', False, "Don't pass --error-deprecated on the command line even when running the TestHarness with --error-deprecated")
        params.addParam('no_additional_cli_args', False, "A Boolean indicating that no additional CLI args should be added from the TestHarness. Note: This parameter should be rarely used as it will not pass on additional options such as those related to mpi, threads, distributed mesh, errors, etc.")

        # Valgrind
        params.addParam('valgrind', 'NORMAL', "Set to (NONE, NORMAL, HEAVY) to determine which configurations where valgrind will run.")

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
        if not (params.isValid('input') or params.isValid('command') or params['no_additional_cli_args']):
            raise Exception('One of "input", "command", or "no_additional_cli_args" must be supplied for a RunApp test')

    def getInputFile(self):
        if self.specs.isValid('input'):
            return self.specs['input'].strip()
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
        if options.enable_recover:
            if self.specs.isValid('expect_out') or self.specs.isValid('absent_out') or self.specs['should_crash'] == True:
                self.addCaveats('expect_out RECOVER')
                self.setStatus(self.skip)
                return False

        if self.specs.isValid('executable_pattern') and re.search(self.specs['executable_pattern'], self.specs['executable']) == None:
            self.addCaveats('EXECUTABLE PATTERN')
            self.setStatus(self.skip)
            return False

        if self.specs.isValid('min_threads') or self.specs.isValid('max_threads'):
            if 'NONE' in options._checks['threading'] and self.getThreads(options) > 1:
                self.addCaveats('threading_model=None')
                self.setStatus(self.skip)
                return False

        return True

    def getThreads(self, options):
        #Set number of threads to be used lower bound
        nthreads = max(options.nthreads, int(self.specs['min_threads']))
        #Set number of threads to be used upper bound
        nthreads = min(nthreads, int(self.specs['max_threads']))

        if nthreads > options.nthreads:
            self.addCaveats('min_threads=' + str(nthreads))
        elif nthreads < options.nthreads:
            self.addCaveats('max_threads=' + str(nthreads))

        return nthreads

    def getProcs(self, options):
        if options.parallel == None:
            default_ncpus = 1
        else:
            default_ncpus = options.parallel

        # Raise the floor
        ncpus = max(default_ncpus, int(self.specs['min_parallel']))
        # Lower the ceiling
        ncpus = min(ncpus, int(self.specs['max_parallel']))

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
            self.setStatus(self.fail, 'Application not found')
            return ''

        # If no_additional_cli_args is set to True, return early with a simplified command line ignoring
        # all other TestHarness supplied options.
        if specs['no_additional_cli_args']:
            # TODO: Do error checking for TestHarness options that will be silently ignored
            return os.path.join(specs['test_dir'], specs['executable']) + ' ' + ' '.join(specs['cli_args'])

        # Create the additional command line arguments list
        cli_args = list(specs['cli_args'])

        if (options.parallel_mesh or options.distributed_mesh) and ('--parallel-mesh' not in cli_args or '--distributed-mesh' not in cli_args):
            # The user has passed the parallel-mesh option to the test harness
            # and it is NOT supplied already in the cli-args option
            cli_args.append('--distributed-mesh')

        if '--error' not in cli_args and (not specs["allow_warnings"] or options.error):
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

        if options.colored == False:
            cli_args.append('--color off')

        if options.cli_args:
            cli_args.insert(0, options.cli_args)

        if options.scaling and specs['scale_refine'] > 0:
            cli_args.insert(0, ' -r ' + str(specs['scale_refine']))

        # The test harness should never use GDB backtraces: they don't
        # work well when dozens of expect_err jobs run at the same time.
        cli_args.append('--no-gdb-backtrace')

        # Get the number of processors and threads the Tester requires
        ncpus = self.getProcs(options)
        nthreads = self.getThreads(options)

        if specs['redirect_output'] and ncpus > 1:
            cli_args.append('--keep-cout --redirect-output ' + self.name())

        command = specs['executable'] + ' ' + specs['input_switch'] + ' ' + specs['input'] + ' ' + ' '.join(cli_args)
        if options.valgrind_mode.upper() == specs['valgrind'].upper() or options.valgrind_mode.upper() == 'HEAVY' and specs['valgrind'].upper() == 'NORMAL':
            command = 'valgrind --suppressions=' + os.path.join(specs['moose_dir'], 'python', 'TestHarness', 'suppressions', 'errors.supp') + ' --leak-check=full --tool=memcheck --dsymutil=yes --track-origins=yes --demangle=yes -v ' + command
        elif nthreads > 1:
            command = command + ' --n-threads=' + str(nthreads)

        if self.force_mpi or options.parallel or ncpus > 1:
            command = self.mpi_command + ' -n ' + str(ncpus) + ' ' + command

        return command

    def testFileOutput(self, moose_dir, options, output):
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
            if custom_module.custom_evaluation(output):
                return errors
            else:
                errors += "#"*80 + "\n\n" + "Custom evaluation failed.\n"
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
                    have_expected_out = util.checkOutputForLiteral(output, specs[param])
                    match_type = 'literal'
                else:
                    have_expected_out = util.checkOutputForPattern(output, specs[param])
                    match_type = 'pattern'

                # Exclusive OR test
                if attr['error_missing'] ^ have_expected_out:
                    reason = attr['reason']
                    errors += "#"*80 + "\n\n" + attr['message'].format(match_type) + "\n\n" + specs[param] + "\n"
                    break

        if reason != '':
            self.setStatus(self.fail, reason)

        return errors

    def testExitCodes(self, moose_dir, options, output):
        # Don't do anything if we already have a status set
        reason = ''
        if self.isNoStatus():
            specs = self.specs
            # We won't pay attention to the ERROR strings if EXPECT_ERR is set (from the derived class)
            # since a message to standard error might actually be a real error.  This case should be handled
            # in the derived class.
            if options.valgrind_mode == '' and not specs.isValid('expect_err') and len( [x for x in filter( lambda x: x in output, specs['errors'] )] ) > 0:
                reason = 'ERRMSG'
            elif self.exit_code == 0 and specs['should_crash'] == True:
                reason = 'NO CRASH'
            elif self.exit_code != 0 and specs['should_crash'] == False:
                # Let's look at the error code to see if we can perhaps further split this out later with a post exam
                reason = 'CRASH'
            # Valgrind runs
            elif self.exit_code == 0 and self.shouldExecute() and options.valgrind_mode != '' and 'ERROR SUMMARY: 0 errors' not in output:
                reason = 'MEMORY ERROR'

            if reason != '':
                self.setStatus(self.fail, str(reason))
                return "\n\nExit Code: " + str(self.exit_code)

        # Return anything extra here that we want to tack onto the Output for when it gets printed later
        return ''

    def processResults(self, moose_dir, options, output):
        """
        Wrapper method for testFileOutput.

        testFileOutput does not set a success status, while processResults does.
        For testers that are RunApp types, they will call this method (processResults).

        Other tester types (like exodiff) will call testFileOutput. This is to prevent
        derived testers from having a successfull status set, before actually running
        the derived processResults method.

        # TODO: because RunParallel is now setting every successful status message,
                refactor testFileOutput and processResults.
        """
        output += self.testFileOutput(moose_dir, options, output)
        output += self.testExitCodes(moose_dir, options, output)

        return output
