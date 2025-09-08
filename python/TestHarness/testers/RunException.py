# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness import util
from RunApp import RunApp


class RunException(RunApp):

    @staticmethod
    def validParams():
        params = RunApp.validParams()

        params.addParam(
            "expect_err",
            "A regular expression or literal string that must occur in the output (see match_literal). (Test may terminate unexpectedly and be considered passing)",
        )
        params.addParam(
            "expect_assert",
            "DEBUG MODE ONLY: A regular expression that must occur in the output. (Test may terminate unexpectedly and be considered passing)",
        )
        params.addParam(
            "should_crash",
            True,
            "Indicates that the test is expected to crash or otherwise terminate early",
        )
        params.addParam("expect_exit_code", "An integer exit code to expect")

        # RunException tests executed in parallel need to have their output redirected to a file, and examined individually
        params["redirect_output"] = True

        params["recover"] = False
        params["restep"] = False

        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)
        if (
            params.isValid("expect_err") == False
            and params.isValid("expect_assert") == False
        ):
            raise RuntimeError(
                'Either "expect_err" or "expect_assert" must be supplied in RunException'
            )

    def checkRunnable(self, options):
        # We seem to have issues with --redirect-output causing
        # "Inappropriate ioctl for device (25)" errors, so if this test
        # requires more procs, we can't run it
        if options.hpc and int(self.specs["min_parallel"] > 1):
            self.addCaveats("hpc max_cpus=1")
            return False
        return RunApp.checkRunnable(self, options)

    def prepare(self, options):
        files = self.getRedirectedOutputFiles(options)
        if files:
            util.deleteFilesAndFolders(self.getTestDir(), files, False)

    def getOutputFiles(self, options):
        return super().getOutputFiles(options) + self.getRedirectedOutputFiles(options)

    def testExitCodes(self, options, exit_code, runner_output):
        reason = super().testExitCodes(options, exit_code, runner_output)

        if not reason:
            specs = self.specs
            if (
                specs.isValid("expect_exit_code")
                and exit_code != specs["expect_exit_code"]
            ):
                self.setStatus(self.fail, "WRONG EXIT CODE")
                reason = f'\nExit code {exit_code} != {specs["expect_exit_code"]}'

        return reason
