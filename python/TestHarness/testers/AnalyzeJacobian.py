# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
from TestHarness import util
from TestHarness.testers import FileTester


class AnalyzeJacobian(FileTester):

    @staticmethod
    def validParams():
        params = FileTester.validParams()
        params.addRequiredParam("input", "The input file to use for this test.")
        params.addParam("test_name", "The name of the test - populated automatically")
        params.addParam(
            "expect_out",
            "A regular expression that must occur in the input in order for the test to be considered passing.",
        )
        params.addParam("resize_mesh", False, "Resize the input mesh")
        params.addParam(
            "off_diagonal", True, "Also test the off-diagonal Jacobian entries"
        )
        params.addParam("mesh_size", 1, "Resize the input mesh")

        params["capture_perf_graph"] = False

        return params

    def __init__(self, name, params):
        FileTester.__init__(self, name, params)

    def getOutputFiles(self, options):
        # analyzejacobian.py outputs files prefixed with the input file name
        return super().getOutputFiles(options) + [self.specs["input"]]

    def prepare(self, options):
        # We do not know what file(s) analyzejacobian.py produces
        return

    def getCommand(self, options):
        specs = self.specs
        # Create the command line string to run
        command = os.path.join(
            self.getMoosePythonDir(), "jacobiandebug", "analyzejacobian.py"
        )

        # Check for built application
        if not options.dry_run and not os.path.exists(command):
            print("Application not found: " + str(specs["executable"]))
            sys.exit(1)

        mesh_options = " -m %s" % options.method
        if specs["resize_mesh"]:
            mesh_options += " -r -s %d" % specs["mesh_size"]

        if not specs["off_diagonal"]:
            mesh_options += " -D"

        command += (
            mesh_options + " " + specs["input"] + " -e " + specs["executable"] + " "
        )
        if len(specs["cli_args"]):
            command += '--cli-args "' + (" ".join(specs["cli_args"]) + '"')

        return command

    def processResults(self, moose_dir, options, exit_code, runner_output):
        reason = ""
        specs = self.specs
        if specs.isValid("expect_out"):
            out_ok = util.checkOutputForPattern(runner_output, specs["expect_out"])
            if out_ok and exit_code != 0:
                reason = "OUT FOUND BUT CRASH"
            elif not out_ok:
                reason = "NO EXPECTED OUT"
        if reason == "":
            if exit_code != 0:
                reason = "CRASH"

        if reason != "":
            self.setStatus(self.fail, reason)

        return ""

    def checkRunnable(self, options):
        # We cannot rely on an external script running things within HPC
        if options.hpc:
            self.addCaveats("hpc unsupported")
            self.setStatus(self.skip)
            return False

        # This doesn't pass valgrind arguments
        if options.valgrind_mode:
            self.addCaveats("valgrind=false")
            self.setStatus(self.skip)
            return False

        return FileTester.checkRunnable(self, options)
