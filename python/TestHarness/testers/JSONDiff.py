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
from mooseutils import JSONDiffer

class JSONDiff(RunApp):

    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addRequiredParam('jsondiff',   [], "A list of XML files to compare.")
        params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
        params.addParam('skip_keys', [], "A list of keys to skip in the JSON comparison")
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

    def prepare(self, options):
        if self.specs['delete_output_before_running'] == True:
            util.deleteFilesAndFolders(self.getTestDir(), self.specs['jsondiff'])

    def processResults(self, moose_dir, options, output):
        output += self.testFileOutput(moose_dir, options, output)
        self.testExitCodes(moose_dir, options, output)

        # Skip
        specs = self.specs

        if self.isFail() or specs['skip_checks']:
            return output

        # Don't Run XMLDiff on Scaled Tests
        if options.scaling and specs['scale_refine']:
            return output

        # Loop over every file
        for file in specs['jsondiff']:

            # Error if gold file does not exist
            if not os.path.exists(os.path.join(self.getTestDir(), specs['gold_dir'], file)):
                output += "File Not Found: " + os.path.join(self.getTestDir(), specs['gold_dir'], file)
                self.setStatus(self.fail, 'MISSING GOLD FILE')
                break

            # Perform diff
            else:
                for file in self.specs['jsondiff']:
                    gold = os.path.join(self.getTestDir(), specs['gold_dir'], file)
                    test = os.path.join(self.getTestDir(), file)

                    differ = JSONDiffer(gold, test, skip_keys=specs['skip_keys'])

                    # Print the results of the Jsondiff whether it passed or failed.
                    output += differ.message() + '\n'

                    if differ.fail():
                        self.setStatus(self.diff, 'JSONDIFF')
                        break

        return output
