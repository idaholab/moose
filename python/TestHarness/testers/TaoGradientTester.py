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

        # Disable with valgrind
        params.valid["valgrind"] = "NONE"
        # No recover or restep
        params["recover"] = False
        params["restep"] = False
        # TAO implementation does not currently support threading
        params["max_threads"] = 1

        return params

    def __init__(self, name, params):
        super().__init__(name, params)

        # Require opt mode
        if self.specs["capabilities"]:
            self.specs["capabilities"] = "(" + self.specs["capabilities"] + ") & "
        self.specs["capabilities"] += "method=opt"

    def getCommand(self, options):
        # Get the base command from RunApp
        cmd = super().getCommand(options)

        # Build PETSc options
        petsc_options = [
            ("-tao_max_it", "1"),
            ("-tao_fd_test", "true"),
            ("-tao_test_gradient", "true"),
            ("-tao_fd_gradient", "false"),
            ("-tao_ls_type", "unit"),
        ]
        if (tao_fd_delta := self.specs["tao_fd_delta"]) is not None:
            petsc_options.append(("-tao_fd_delta", f"{tao_fd_delta}"))

        # Append executioner options
        executioner_options = [
            "tao_solver=taobncg",
            f"petsc_options_iname='" + " ".join([v[0] for v in petsc_options]) + "'",
            f"petsc_options_value='" + " ".join([v[1] for v in petsc_options]) + "'",
            "petsc_options='-tao_test_gradient_view'",
            "verbose=true",
        ]
        cmd += " " + " ".join(f"Executioner/{v}" for v in executioner_options)

        return cmd

    def processResults(self, moose_dir, options, exit_code, runner_output):
        # Check parent for failures (capabilities, exit code)
        output = super().processResults(moose_dir, options, exit_code, runner_output)
        if self.isFail():
            return output

        fail_reason = None

        # Match all gradient test outputs in the output
        if maxnorm_matches := self.maxnorm_re.findall(runner_output):
            assert len(maxnorm_matches) > 0
            match = maxnorm_matches[0]
            maxnorm_val = float(match.rstrip("."))
            max_rel_tol = self.specs["max_rel_tol"]

            if maxnorm_val > max_rel_tol:
                fail_reason = "MAX-NORM TOO LARGE"
                output = f"Gradient test max-norm too large: {maxnorm_val:.2e} > tol={max_rel_tol:.2e}"
            else:
                output = f"Gradient test max-norm check successful: {maxnorm_val:.2e} < tol={max_rel_tol:.2e}"
        else:
            fail_reason = "EXPECTED OUTPUT NOT FOUND"
            output = "Expected output for 'max-norm ||G - Gfd||/||G||' not found"

        if fail_reason:
            self.setStatus(self.fail, fail_reason)

        return output
