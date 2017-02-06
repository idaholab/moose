from RunApp import RunApp
from util import runCommand
import os

class CheckFiles(RunApp):

    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam('check_files', [], "A list of files that MUST exist.")
        params.addParam('check_not_exists', [], "A list of files that must NOT exist.")
        params.addParam('file_expect_out', "A regular expression that must occur in all of the check files in order for the test to be considered passing.")
        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

  def prepare(self, options):
    if self.specs['delete_output_before_running'] == True:
            self.deleteFilesAndFolders(self.specs['test_dir'], self.specs['check_files'] + self.specs['check_not_exists'], self.specs['delete_output_folders'])

    def processResults(self, moose_dir, retcode, options, output):
        (reason, output) = RunApp.processResults(self, moose_dir, retcode, options, output)

        specs = self.specs
        if reason != '' or specs['skip_checks']:
            return (reason, output)

        if reason == '':
            # if still no errors, check other files (just for existence)
            for file in self.specs['check_files']:
                if not os.path.isfile(os.path.join(self.specs['test_dir'], file)):
                    reason = 'MISSING FILES'
                    break
            for file in self.specs['check_not_exists']:
                if os.path.isfile(os.path.join(self.specs['test_dir'], file)):
                    reason = 'UNEXPECTED FILES'
                    break

            # if still no errors, check that all the files contain the file_expect_out expression
            if reason == '':
                if self.specs.isValid('file_expect_out'):
                    for file in self.specs['check_files']:
                        fid = open(os.path.join(self.specs['test_dir'], file), 'r')
                        contents = fid.read()
                        fid.close()
                        if not self.checkOutputForPattern(contents, self.specs['file_expect_out']):
                            reason = 'NO EXPECTED OUT IN FILE'
                            break

        return (reason, output)
