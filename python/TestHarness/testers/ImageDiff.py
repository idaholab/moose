import os
import sys

from RunPythonApp import RunPythonApp
from mooseutils.ImageDiffer import ImageDiffer

class ImageDiff(RunPythonApp):


    @staticmethod
    def validParams():
        params = RunPythonApp.validParams()
        params.addRequiredParam('imagediff', [], 'A list of files to compare against the gold.')
        params.addParam('gold_dir', 'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
        params.addParam('allowed', 0.98, "Absolute zero cutoff used in exodiff comparisons.")
        params.addParam('delete_output_before_running', True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")
        params.addParam('allowed_linux', "Absolute zero cuttoff used for linux machines, if not provided 'allowed' is used.")
        params.addParam('allowed_darwin', "Absolute zero cuttoff used for Mac OS (Darwin) machines, if not provided 'allowed' is used.")
        return params

    def __init__(self, name, params):
        RunPythonApp.__init__(self, name, params)

    def prepare(self, options):
        """
        Cleans up image files from previous execution
        """
        if self.specs['delete_output_before_running'] == True:
            for file in self.specs['imagediff']:
                try:
                    os.remove(os.path.join(self.specs['test_dir'], file))
                except:
                    pass

    def processResults(self, moose_dir, retcode, options, output):
        """
        Perform image diff
        """

        # Call base class processResults
        (reason, output) = RunPythonApp.processResults(self, moose_dir, retcode, options, output)
        if reason:
            return (reason, output)

        # Loop through files
        specs = self.specs
        for filename in specs['imagediff']:

            # Error if gold file does not exist
            if not os.path.exists(os.path.join(specs['test_dir'], specs['gold_dir'], filename)):
                output += "File Not Found: " + os.path.join(specs['test_dir'], specs['gold_dir'], filename)
                reason = 'MISSING GOLD FILE'
                break

            # Perform diff
            else:

                output = 'Running ImageDiffer.py'
                gold = os.path.join(specs['test_dir'], specs['gold_dir'], filename)
                test = os.path.join(specs['test_dir'], filename)

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
                    reason = 'IMAGEDIFF'
                    break

        # Return the reason and command output
        return (reason, output)
