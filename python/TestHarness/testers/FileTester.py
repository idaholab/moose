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

# Classes that derive from this class are expected to write
# output files. The Tester::getOutputFiles() method should
# be implemented for all derived classes.
class FileTester(RunApp):
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam('gold_dir', 'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
        params.addParam('abs_zero',       1e-10, "Absolute zero cutoff used in exo/csvdiff comparisons.")
        params.addParam('rel_err',       5.5e-6, "Relative error value used in exo/csvdiff comparisons.")
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

    def prepare(self, options):
        if self.specs['delete_output_before_running']:
            util.deleteFilesAndFolders(self.getTestDir(), self.getOutputFiles(), self.specs['delete_output_folders'])
