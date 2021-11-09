#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from FileTester import FileTester
import os
import sys
from mooseutils.ImageDiffer import ImageDiffer

class ImageDiff(FileTester):

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addRequiredParam('imagediff', [], 'A list of files to compare against the gold.')
        params.addParam('allowed', 0.98, "Absolute zero cutoff used in exodiff comparisons.")
        params.addParam('allowed_linux', "Absolute zero cuttoff used for linux machines, if not provided 'allowed' is used.")
        params.addParam('allowed_darwin', "Absolute zero cuttoff used for Mac OS (Darwin) machines, if not provided 'allowed' is used.")
        # We don't want to check for any errors on the screen with this. If there are any real errors then the image test will fail.
        params['errors'] = []
        params['display_required'] = False
        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)
        if self.specs['required_python_packages'] is None:
             self.specs['required_python_packages'] = 'skimage'
        elif 'skimage' not in self.specs['required_python_packages']:
            self.specs['required_python_packages'] += ' skimage'

    def getOutputFiles(self):
        return self.specs['imagediff']

    def processResults(self, moose_dir, options, output):
        """
        Perform image diff
        """

        # Call base class processResults
        output += FileTester.processResults(self, moose_dir, options, output)
        if self.isFail():
            return output

        # Loop through files
        specs = self.specs
        for filename in specs['imagediff']:

            # Error if gold file does not exist
            if not os.path.exists(os.path.join(self.getTestDir(), specs['gold_dir'], filename)):
                output += "File Not Found: " + os.path.join(self.getTestDir(), specs['gold_dir'], filename)
                self.setStatus(self.fail, 'MISSING GOLD FILE')
                break

            # Perform diff
            else:
                output = 'Running ImageDiffer.py'
                gold = os.path.join(self.getTestDir(), specs['gold_dir'], filename)
                test = os.path.join(self.getTestDir(), filename)

                if sys.platform in ['linux', 'linux2']:
                    name = 'allowed_linux'
                elif sys.platform == 'darwin':
                    name = 'allowed_darwin'
                allowed = specs[name] if specs.isValid(name) else specs['allowed']
                differ = ImageDiffer(gold, test, allowed=allowed)

                # Update golds (e.g., uncomment this to re-gold for new system or new defaults)
                #import shutil; shutil.copy(test, gold)

                output += differ.message()
                if differ.fail():
                    self.setStatus(self.diff, 'IMAGEDIFF')
                    break

        return output
