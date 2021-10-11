#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from RunApp import RunApp
from TestHarness import util
import os

# Classes that derive from this class are expected to write
# output files. The Tester::getOutputFiles() method should
# be implemented for all derived classes.
class FileTester(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam('gold_dir', 'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
        params.addParam('abs_zero',       1e-10, "Absolute zero cutoff used in exodiff comparisons.")
        params.addParam('rel_err',       5.5e-6, "Relative error value used in exodiff comparisons.")
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

    def prepare(self, options):
        if self.specs['delete_output_before_running']:
            util.deleteFilesAndFolders(self.getTestDir(), self.getOutputFiles(), self.specs['delete_output_folders'])

    def testFileOutput(self, moose_dir, options, output):
        """ Set a failure status for expressions found in output """
        reason = None
        specs = self.specs

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
                    output += "#"*80 + "\n\n" + attr['message'].format(match_type) + "\n\n" + specs[param] + "\n"
                    break

        if reason:
            self.setStatus(self.fail, reason)

        return output

    def testExitCodes(self, moose_dir, options, output):
        # Don't do anything if we already have a status set
        reason = None
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

            if reason:
                self.setStatus(self.fail, reason)
                output += "\n\nExit Code: " + str(self.exit_code)

        # Return anything extra here that we want to tack onto the Output for when it gets printed later
        return output

    def testForGoldFile(self, moose_dir, output):
        if self.getOutputFiles():
            for file in self.getOutputFiles():
                if not os.path.exists(os.path.join(self.getTestDir(), self.specs['gold_dir'], file)):
                    output += "File Not Found: " + os.path.join(self.getTestDir(), self.specs['gold_dir'], file)
                    self.setStatus(self.fail, 'MISSING GOLD FILE')
                    break
        return output

    def processResults(self, moose_dir, options, output):
        """ Run basic tests common for RunApp type testers """
        output = self.testForGoldFile(moose_dir, output)
        output = self.testFileOutput(moose_dir, options, output)
        output = self.testExitCodes(moose_dir, options, output)
        return output
