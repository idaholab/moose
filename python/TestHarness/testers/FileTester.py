from RunApp import RunApp
import util
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
        if self.specs['delete_output_before_running'] == True:
            util.deleteFilesAndFolders(self.specs['test_dir'], self.getOutputFiles(), self.specs['delete_output_folders'])

    def processResults(self, moose_dir, retcode, options, output):
        return RunApp.processResults(self, moose_dir, retcode, options, output)
