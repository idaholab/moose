# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness.testers.FileTester import FileTester
from TestHarness import util
import os


class CheckFiles(FileTester):

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addParam("check_files", [], "A list of files that MUST exist.")
        params.addParam("check_not_exists", [], "A list of files that must NOT exist.")
        params.addParam(
            "file_expect_out",
            "A regular expression that must occur in all of the check files in order for the test to be considered passing.",
        )
        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)

        # Make sure that either input or command is supplied
        if not (params.isValid("check_files") or params.isValid("check_not_exists")):
            raise Exception(
                'Either "check_files" or "check_not_exists" must be supplied for a CheckFiles test'
            )

    def getOutputFiles(self, options):
        return (
            super().getOutputFiles(options)
            + self.specs["check_files"]
            + self.specs["check_not_exists"]
        )

    def processResults(self, moose_dir, options, exit_code, runner_output):
        output = super().processResults(moose_dir, options, exit_code, runner_output)

        specs = self.specs

        if self.isFail() or specs["skip_checks"]:
            return output
        else:
            reason = ""
            # if still no errors, check other files (just for existence)
            errors = []
            for file in self.specs["check_files"]:
                full_path = os.path.abspath(os.path.join(self.getTestDir(), file))
                if os.path.isfile(full_path):
                    errors.append('File "' + full_path + '" exists.')
                else:
                    errors.append('File "' + full_path + '" does not exist but should.')
                    reason = "MISSING FILES"
            for file in self.specs["check_not_exists"]:
                full_path = os.path.abspath(os.path.join(self.getTestDir(), file))
                if os.path.isfile(full_path):
                    errors.append('File "' + full_path + '" exists but should not.')
                    reason = "UNEXPECTED FILES"
                else:
                    errors.append('File "' + full_path + '" does not exist - ok.')

            if reason != "":
                output += "\n" + "\n".join(errors)
            else:
                # if still no errors, check that all the files contain the file_expect_out expression
                if self.specs.isValid("file_expect_out"):
                    for file in self.specs["check_files"]:
                        fid = open(os.path.join(self.getTestDir(), file), "r")
                        contents = fid.read()
                        fid.close()
                        if not util.checkOutputForPattern(
                            contents, self.specs["file_expect_out"]
                        ):
                            reason = "NO EXPECTED OUT IN FILE"
                            break

        # populate status bucket
        if reason != "":
            self.setStatus(self.fail, reason)

        return output

    def checkRunnable(self, options):
        # We cannot reliably check if files do not exist with a networked file system
        if options.hpc and self.specs["check_not_exists"]:
            self.addCaveats("hpc unsupported")
            self.setStatus(self.skip)
            return False

        return super().checkRunnable(options)
