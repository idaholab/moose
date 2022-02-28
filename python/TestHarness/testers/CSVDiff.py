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

class CSVDiff(FileTester):

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addRequiredParam('csvdiff',   [], "A list of files to run CSVDiff on.")
        params.addParam('override_columns',   [], "A list of variable names to customize the CSVDiff tolerances.")
        params.addParam('override_rel_err',   [], "A list of customized relative error tolerances.")
        params.addParam('override_abs_zero',   [], "A list of customized absolute zero tolerances.")
        params.addParam('comparison_file', "Use supplied custom comparison config file.")

        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)

    def getOutputFiles(self):
        return self.specs['csvdiff']

    # Check that override parameter lists are the same length
    def checkRunnable(self, options):
        if ((len(self.specs['override_columns']) != len(self.specs['override_rel_err']))
        or (len(self.specs['override_columns']) != len(self.specs['override_abs_zero']))
        or (len(self.specs['override_rel_err']) != len(self.specs['override_abs_zero']))):
           self.setStatus(self.fail, 'Override inputs not the same length')
           return False
        return FileTester.checkRunnable(self, options)

    def processResultsCommand(self, moose_dir, options):
        commands = []

        for file in self.specs['csvdiff']:
            csvdiff = [os.path.join(self.specs['moose_python_dir'], 'mooseutils', 'csvdiff.py')]

            # Due to required positional nargs with the ability to support custom positional args (--argument), we need to specify the required ones first
            csvdiff.append(os.path.join(self.getTestDir(), self.specs['gold_dir'], file) + ' ' + os.path.join(self.getTestDir(), file))

            if self.specs.isValid('rel_err'):
                csvdiff.append('--relative-tolerance %s' % (self.specs['rel_err']))

            if self.specs.isValid('abs_zero'):
                csvdiff.append('--abs-zero %s' % (self.specs['abs_zero']))

            if self.specs.isValid('comparison_file'):
                comparison_file = os.path.join(self.getTestDir(), self.specs['comparison_file'])
                if os.path.exists(comparison_file):
                    csvdiff.append('--comparison-file %s' % (comparison_file))
                else:
                    self.setStatus(self.fail, 'MISSING COMPARISON FILE')
                    return commands

            if self.specs.isValid('override_columns'):
                csvdiff.append('--custom-columns %s' % (' '.join(self.specs['override_columns'])))

            if self.specs.isValid('override_rel_err'):
                csvdiff.append('--custom-rel-err %s' % (' '.join(self.specs['override_rel_err'])))

            if self.specs.isValid('override_abs_zero'):
                csvdiff.append('--custom-abs-zero %s' % (' '.join(self.specs['override_abs_zero'])))

            commands.append(' '.join(csvdiff))

        return commands

    def processResults(self, moose_dir, options, output):
        output += FileTester.processResults(self, moose_dir, options, output)

        if self.isFail() or self.specs['skip_checks']:
            return output

        # Don't Run CSVDiff on Scaled Tests
        if options.scaling and self.specs['scale_refine']:
            return output

        # Make sure that all of the CSVDiff files are actually available
        for file in self.specs['csvdiff']:
            if not os.path.exists(os.path.join(self.getTestDir(), self.specs['gold_dir'], file)):
                output += "File Not Found: " + os.path.join(self.getTestDir(), self.specs['gold_dir'], file)
                self.setStatus(self.fail, 'MISSING GOLD FILE')
                break

        if not self.isFail():
            # Retrieve the commands
            commands = self.processResultsCommand(moose_dir, options)

            for command in commands:
                exo_output = util.runCommand(command)
                output += 'Running csvdiff: ' + command + '\n' + exo_output
                if not "Files are the same" in exo_output:
                    self.setStatus(self.diff, 'CSVDIFF')
                    break

        return output
