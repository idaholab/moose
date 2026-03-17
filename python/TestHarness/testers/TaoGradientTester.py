# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from RunApp import RunApp
import re


class TaoGradientTester(RunApp):
    maxnorm_re = re.compile(r"max-norm\s+\|\|G\s*-\s*Gfd\|\|/\|\|G\|\|\s*=\s*([^,\s]+)")

    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam(
            "max_rel_tol",
            1e-5,
            "Tolerance for the relative max-norm ||G - Gfd||/||G||. Should be near "
            "zero for a correct gradient.",
        )
        params.addParam(
            "tao_fd_delta",
            None,
            "Finite difference step size for the gradient test (-tao_fd_delta). "
            "If not specified, TAO uses its default.",
        )
        params.addParam(
            "only_first_gradient",
            True,
            "Check only the first gradient comparison. If False, "
            "all gradient comparisons in the output are checked.",
        )

        # Disable with valgrind
        params.valid["valgrind"] = "NONE"
        # No recover or restep
        params["recover"] = False
        params["restep"] = False
        # TAO implementation does not currently support threading
        params["max_threads"] = 1

        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

        # Require opt mode
        if self.specs["capabilities"]:
            self.specs["capabilities"] = "(" + self.specs["capabilities"] + ") & "
        self.specs["capabilities"] += "method=opt"

    def getCommand(self, options):
        # Get the base command from RunApp
        cmd = RunApp.getCommand(self, options)

        petsc_iname = [
            "-tao_max_it",
            "-tao_fd_test",
            "-tao_test_gradient",
            "-tao_fd_gradient",
            "-tao_ls_type",
        ]
        petsc_value = ["1", "true", "true", "false", "unit"]

        specs = self.specs
        if specs["tao_fd_delta"] is not None:
            petsc_iname.append("-tao_fd_delta")
            petsc_value.append(str(specs["tao_fd_delta"]))

        cmd += f" Executioner/tao_solver=taobncg"
        cmd += " Executioner/petsc_options_iname='" + " ".join(petsc_iname) + "'"
        cmd += " Executioner/petsc_options_value='" + " ".join(petsc_value) + "'"
        cmd += " Executioner/petsc_options='-tao_test_gradient_view'"
        cmd += " Executioner/verbose=true"

        return cmd

    def processResults(self, moose_dir, options, exit_code, runner_output):
        output = ""

        # Match all gradient test outputs in the output
        maxnorm_matches = self.maxnorm_re.findall(runner_output)

        reason = "EXPECTED OUTPUT for 'max-norm ||G - Gfd||/||G||' NOT FOUND"

        if maxnorm_matches:
            # If only_first_gradient, check just the first; otherwise check all
            if self.specs["only_first_gradient"]:
                matches = [maxnorm_matches[0]]
            else:
                matches = maxnorm_matches

            reason = ""
            for maxnorm_str in matches:
                maxnorm_val = float(maxnorm_str.rstrip("."))
                max_rel_tol = self.specs["max_rel_tol"]

                reason = ""
                if maxnorm_val > max_rel_tol:
                    reason = f"GRADIENT TEST: MAX-NORM TOO LARGE ({maxnorm_val:.2e} > tol={max_rel_tol:.2e})"

                # Break on first failure
                if reason:
                    break

        if reason:
            self.setStatus(self.fail, reason)

        return output
