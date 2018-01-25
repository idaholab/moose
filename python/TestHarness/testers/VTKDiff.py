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
from TestHarness.XMLDiffer import XMLDiffer

class VTKDiff(RunApp):

    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addRequiredParam('vtkdiff',   [], "A list of files to exodiff.")
        params.addParam('gold_dir',      'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
        params.addParam('abs_zero',       1e-10, "Absolute zero cutoff used in exodiff comparisons.")
        params.addParam('rel_err',       5.5e-6, "Relative error value used in exodiff comparisons.")
        params.addParam('ignored_attributes',  [], "Ignore e.g. type and/or version in sample XML block <VTKFile type=\"Foo\" version=\"0.1\">")

        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

    def prepare(self, options):
        if self.specs['delete_output_before_running'] == True:
            util.deleteFilesAndFolders(self.specs['test_dir'], self.specs['vtkdiff'])

    def processResults(self, moose_dir, options, output):
        RunApp.testFileOutput(self, moose_dir, options, output)

        # Skip
        specs = self.specs

        if self.getStatus() == self.bucket_fail or specs['skip_checks']:
            return output

        # Don't Run VTKDiff on Scaled Tests
        if options.scaling and specs['scale_refine']:
            return output

        # Loop over every file
        for file in specs['vtkdiff']:

            # Error if gold file does not exist
            if not os.path.exists(os.path.join(specs['test_dir'], specs['gold_dir'], file)):
                output += "File Not Found: " + os.path.join(specs['test_dir'], specs['gold_dir'], file)
                self.setStatus('MISSING GOLD FILE', self.bucket_fail)
                break

            # Perform diff
            else:
                for file in self.specs['vtkdiff']:
                    gold = os.path.join(specs['test_dir'], specs['gold_dir'], file)
                    test = os.path.join(specs['test_dir'], file)

                    # We always ignore the header_type attribute, since it was
                    # introduced in VTK 7 and doesn't seem to be important as
                    # far as Paraview is concerned.
                    specs['ignored_attributes'].append('header_type')

                    differ = XMLDiffer(gold, test, abs_zero=specs['abs_zero'], rel_tol=specs['rel_err'], ignored_attributes=specs['ignored_attributes'])

                    # Print the results of the VTKDiff whether it passed or failed.
                    output += differ.message() + '\n'

                    if differ.fail():
                        self.addCaveats('VTKDIFF')
                        self.setStatus(self.bucket_skip.status, self.bucket_skip)
                        break

        # If status is still pending, then it is a passing test
        if self.getStatus() == self.bucket_pending:
            self.setStatus(self.success_message, self.bucket_success)

        return output
