#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from FileTester import FileTester
from TestHarness.CSVDiffer import CSVDiffer

class CSVDiff(FileTester):

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addRequiredParam('csvdiff',   [], "A list of files to run CSVDiff on.")
        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)

    def getOutputFiles(self):
        return self.specs['csvdiff']

    def processResults(self, moose_dir, options, output):
        FileTester.processResults(self, moose_dir, options, output)

        specs = self.specs

        if self.getStatus() == self.bucket_fail or specs['skip_checks']:
            return output

        # Don't Run CSVDiff on Scaled Tests
        if options.scaling and specs['scale_refine']:
            self.addCaveats('SCALING=True')
            self.setStatus(self.bucket_skip.status, self.bucket_skip)
            return output

        if len(specs['csvdiff']) > 0:
            differ = CSVDiffer(specs['test_dir'], specs['csvdiff'], specs['abs_zero'], specs['rel_err'], specs['gold_dir'])
            msg = differ.diff()
            output += 'Running CSVDiffer.py\n' + msg
            if msg != '':
                if msg.find("Gold file does not exist!") != -1:
                    self.setStatus('MISSING GOLD FILE', self.bucket_fail)
                elif msg.find("File does not exist!") != -1:
                    self.setStatus('FILE DOES NOT EXIST', self.bucket_fail)
                else:
                    self.setStatus('CSVDIFF', self.bucket_diff)
                return output

        self.setStatus(self.success_message, self.bucket_success)
        return output
