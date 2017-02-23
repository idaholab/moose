from FileTester import FileTester
import util
import os

class Exodiff(FileTester):

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addRequiredParam('exodiff',   [], "A list of files to exodiff.")
        params.addParam('exodiff_opts',      [], "Additional arguments to be passed to invocations of exodiff.")
        params.addParam('custom_cmp',            "Custom comparison file")
        params.addParam('use_old_floor',  False, "Use Exodiff old floor option")
        params.addParam('map',  True, "Use geometrical mapping to match up elements.  This is usually a good idea because it makes files comparable between runs with Serial and Parallel Mesh.")

        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)

    def getOutputFiles(self):
        return self.specs['exodiff']

    def processResultsCommand(self, moose_dir, options):
        commands = []

        for file in self.specs['exodiff']:
            custom_cmp = ''
            old_floor = ''
            if self.specs.isValid('custom_cmp'):
                custom_cmp = ' -f ' + os.path.join(self.specs['test_dir'], self.specs['custom_cmp'])
            if self.specs['use_old_floor']:
                old_floor = ' -use_old_floor'

            if self.specs['map']:
                map_option = ' -m '
            else:
                map_option = ' '

            commands.append(os.path.join(moose_dir, 'framework', 'contrib', 'exodiff', 'exodiff') + map_option + custom_cmp + ' -F' + ' ' + str(self.specs['abs_zero']) \
                            + old_floor + ' -t ' + str(self.specs['rel_err']) + ' ' + ' '.join(self.specs['exodiff_opts']) + ' ' \
                            + os.path.join(self.specs['test_dir'], self.specs['gold_dir'], file) + ' ' + os.path.join(self.specs['test_dir'], file))

        return commands

    def processResults(self, moose_dir, retcode, options, output):
        output = FileTester.processResults(self, moose_dir, retcode, options, output)

        if self.getStatus() == self.bucket_fail or self.specs['skip_checks']:
            return output

        # Don't Run Exodiff on Scaled Tests
        if options.scaling and self.specs['scale_refine']:
            self.success_message = "SCALED"
            self.setStatus(self.getSuccessMessage(), self.bucket_success)
            return output

        # Make sure that all of the Exodiff files are actually available
        for file in self.specs['exodiff']:
            if not os.path.exists(os.path.join(self.specs['test_dir'], self.specs['gold_dir'], file)):
                output += "File Not Found: " + os.path.join(self.specs['test_dir'], self.specs['gold_dir'], file)
                self.setStatus('MISSING GOLD FILE', self.bucket_fail)
                break

        if self.getStatus() != self.bucket_fail:
            # Retrieve the commands
            commands = self.processResultsCommand(moose_dir, options)

            for command in commands:
                exo_output = util.runCommand(command)

                output += 'Running exodiff: ' + command + '\n' + exo_output + ' ' + ' '.join(self.specs['exodiff_opts'])

                if ('different' in exo_output or 'ERROR' in exo_output) and not "Files are the same" in exo_output:
                    self.setStatus('EXODIFF', self.bucket_diff)
                    break

        # If status is still pending, then it is a passing test
        if self.getStatus() == self.bucket_pending:
            self.setStatus(self.success_message, self.bucket_success)

        return output
