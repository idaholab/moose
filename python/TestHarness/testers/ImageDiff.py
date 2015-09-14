import os
from Tester import Tester
from ImageDiffer import ImageDiffer

class ImageDiff(Tester):


    @staticmethod
    def validParams():
        params = Tester.validParams()
        params.addRequiredParam('imagediff', [], 'A list of files to compare against the gold.')
        params.addRequiredParam('input', "The python input file to use for this test.")

        params.addParam('gold_dir', 'gold', "The directory where the \"golden standard\" files reside relative to the TEST_DIR: (default: ./gold/)")
        params.addParam('abs_zero', 1e-10, "Absolute zero cutoff used in exodiff comparisons.")
        params.addParam('delete_output_before_running', True, "Delete pre-existing output files before running test. Only set to False if you know what you're doing!")
        return params

    def __init__(self, name, params):
        Tester.__init__(self, name, params)

    # Cleans up image files from previous execution
    def prepare(self):
        if self.specs['delete_output_before_running'] == True:
            for file in self.specs['imagediff']:
                try:
                    os.remove(os.path.join(self.specs['test_dir'], file))
                except:
                    pass


    # Returns the python script to execute
    def getCommand(self, options):
        return os.path.join(self.specs['test_dir'], self.specs['input'])

    # Perform image diff
    def processResults(self, moose_dir, retcode, options, output):

        # The failure reason
        reason = ''

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
                differ = ImageDiffer(gold, test, abs_zero=specs['abs_zero'])

                if differ.fail():
                    reason = 'IMAGEDIFF'
                    output += differ.message()
                    break

        # Return the reason and command output
        return (reason, output)
