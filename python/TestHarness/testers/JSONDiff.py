#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from FileTester import FileTester
from TestHarness import util
import os
from mooseutils import JSONDiffer

class JSONDiff(FileTester):

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addRequiredParam('jsondiff',   [], "A list of JSON files to compare.")
        params.addParam('skip_keys', [], "A list of keys to skip in the JSON comparison.")
        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)

    def getOutputFiles(self):
        return self.specs['jsondiff']

    def processResults(self, moose_dir, options, output):
        output += FileTester.processResults(self, moose_dir, options, output)

        # Specs
        specs = self.specs

        if self.isFail() or specs['skip_checks']:
            return output

        # Don't Run JSONDiff on Scaled Tests
        if options.scaling and specs['scale_refine']:
            return output

        # Check if files exist
        for file in self.specs['jsondiff']:
            # Get file names and error if not found
            gold_file = os.path.join(self.getTestDir(), specs['gold_dir'], file)
            test_file = os.path.join(self.getTestDir(), file)
            if not os.path.exists(gold_file):
                output += "File Not Found: " + gold_file
                self.setStatus(self.fail, 'MISSING GOLD FILE')
            if not os.path.exists(test_file):
                output += "File Not Found: " + test_file
                self.setStatus(self.fail, 'MISSING OUTPUT FILE')

        if self.isFail():
            return output

        # Loop over every file
        for file in self.specs['jsondiff']:
            # Get file names
            gold_file = os.path.join(self.getTestDir(), specs['gold_dir'], file)
            test_file = os.path.join(self.getTestDir(), file)

            # Run JSONDiffer
            output += 'Running JSONDiff:\n  File 1: {}\n  File 2: {}\n  relative error = {}\n'.format(test_file, gold_file, specs['rel_err'])
            if len(specs['skip_keys']):
                output += '  Skipping: {}\n'.format(specs['skip_keys'])

            differ = JSONDiffer(test_file, gold_file, verbose_level=2, relative_error=specs['rel_err'], skip_keys=specs['skip_keys'])

            # Print the results of the Jsondiff whether it passed or failed.
            output += differ.message() + '\n'

            if differ.fail():
                self.setStatus(self.diff, 'JSONDIFF')

        return output
