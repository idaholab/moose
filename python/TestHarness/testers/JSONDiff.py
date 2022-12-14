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

class JSONDiff(FileTester):

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addRequiredParam('jsondiff',   [], "A list of JSON files to compare.")
        params.addParam('skip_keys', [], "A list of keys to skip in the JSON comparison.")
        params.addParam('keep_system_information', False, "Whether or not to keep the system information as part of the diff.")
        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)
        if self.specs['required_python_packages'] is None:
             self.specs['required_python_packages'] = 'deepdiff'
        elif 'deepdiff' not in self.specs['required_python_packages']:
            self.specs['required_python_packages'] += ' deepdiff'
        # Add system information to skip keys
        if (not self.specs['keep_system_information']):
            self.specs['skip_keys'].extend(['app_name',
                                            'current_time',
                                            'executable',
                                            'executable_time',
                                            'moose_version',
                                            'libmesh_version',
                                            'petsc_version',
                                            'slepc_version'])

    def getOutputFiles(self):
        return self.specs['jsondiff']

    def processResultsCommand(self, moose_dir, options):
        commands = []

        for file in self.specs['jsondiff']:
            gold_file = os.path.join(self.getTestDir(), self.specs['gold_dir'], file)
            test_file = os.path.join(self.getTestDir(), file)

            jsondiff = [os.path.join(self.specs['moose_python_dir'], 'mooseutils', 'jsondiff.py')]
            jsondiff.append(gold_file + ' ' + test_file)
            if self.specs.isValid('rel_err'):
                jsondiff.append('--rel_err %s' % (self.specs['rel_err']))
            if self.specs.isValid('abs_zero'):
                jsondiff.append('--abs_zero %s' % (self.specs['abs_zero']))
            if self.specs.isValid('skip_keys'):
                jsondiff.append('--skip_keys %s' % (' '.join(self.specs['skip_keys'])))
            commands.append(' '.join(jsondiff))

        return commands

    def processResults(self, moose_dir, options, output):
        output += FileTester.processResults(self, moose_dir, options, output)

        if self.isFail() or self.specs['skip_checks']:
            return output

        # Don't Run JSONDiff on Scaled Tests
        if options.scaling and self.specs['scale_refine']:
            return output

        # Check if files exist
        for file in self.specs['jsondiff']:
            # Get file names and error if not found
            gold_file = os.path.join(self.getTestDir(), self.specs['gold_dir'], file)
            test_file = os.path.join(self.getTestDir(), file)
            if not os.path.exists(gold_file):
                output += "File Not Found: " + gold_file
                self.setStatus(self.fail, 'MISSING GOLD FILE')
            if not os.path.exists(test_file):
                output += "File Not Found: " + test_file
                self.setStatus(self.fail, 'MISSING OUTPUT FILE')

        if not self.isFail():
            commands = self.processResultsCommand(moose_dir, options)
            for command in commands:
                exo_output = util.runCommand(command)
                output += 'Running jsondiff: ' + command + '\n' + exo_output
                if not "Files are the same" in exo_output:
                    self.setStatus(self.diff, 'JSONDIFF')

        return output
