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
    @staticmethod
    def validParams():
        params = RunApp.validParams()
        params.addParam(
            "cosine_tol",
            1e-2,
            "Tolerance for |1 - angle_cosine|. The angle cosine between the "
            "hand-coded and finite-difference gradients should be 1.0 for a "
            "perfect gradient.",
        )
        params.addParam(
            "max_norm_tol",
            1e-6,
            "Tolerance for the absolute max-norm ||G - Gfd||. Should be near "
            "zero for a correct gradient.",
        )
        params.addParam(
            "tao_solver",
            "taobncg",
            "TAO solver to use for gradient testing.",
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

        return params

    def __init__(self, name, params):
        RunApp.__init__(self, name, params)

        specs = self.specs

        tao_solver = specs["tao_solver"]

        petsc_iname = "-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_gatol -tao_ls_type"
        petsc_value = "1 true true false 1e10 unit"

        if specs["tao_fd_delta"] is not None:
            petsc_iname += " -tao_fd_delta"
            petsc_value += " " + str(specs["tao_fd_delta"])

        self.specs["cli_args"][:0] = [
            "Executioner/tao_solver=" + tao_solver,
            "Executioner/petsc_options_iname='" + petsc_iname + "'",
            "Executioner/petsc_options_value='" + petsc_value + "'",
            "Executioner/petsc_options='-tao_test_gradient_view'",
            "Executioner/verbose=true",
            "Outputs/exodus=false",
        ]

        # Require opt mode
        if specs["capabilities"]:
            specs["capabilities"] = "(" + specs["capabilities"] + ") & "
        specs["capabilities"] += "method=opt"

    def __strToFloat(self, str):
        """Convert string to float, handling PETSc's nan./inf. formatting."""
        if str == "nan." or str == "inf.":
            return float(str[:-1])
        elif str == "-nan.":
            return float("nan")
        else:
            return float(str)

    def processResults(self, moose_dir, options, exit_code, runner_output):
        output = ""

        # Parse the angle cosine: (Gfd'G)/||Gfd||||G|| = <value>
        cosine_match = re.search(
            r"\(Gfd'G\)/\|\|Gfd\|\|\|\|G\|\|\s*=\s*(\S+)",
            runner_output,
        )

        # Parse the max-norm: ||G - Gfd|| = <value>
        maxnorm_match = re.search(
            r"max-norm\s+\|\|G\s*-\s*Gfd\|\|/\|\|G\|\|\s*=\s*\S+,\s*\|\|G\s*-\s*Gfd\|\|\s*=\s*(\S+)",
            runner_output,
        )

        reason = ""

        if not cosine_match or not maxnorm_match:
            reason = "EXPECTED OUTPUT NOT FOUND"
        else:
            cosine_val = self.__strToFloat(cosine_match.group(1))
            maxnorm_val = self.__strToFloat(maxnorm_match.group(1))

            cosine_err = abs(1.0 - cosine_val)
            if cosine_err > float(self.specs["cosine_tol"]):
                reason = "GRADIENT TEST: ANGLE COSINE {} IS NOT 1 (err={:.2e} > tol={:.2e})".format(
                    cosine_val, cosine_err, float(self.specs["cosine_tol"])
                )

            if maxnorm_val > float(self.specs["max_norm_tol"]):
                if reason:
                    reason += "; "
                reason += "GRADIENT TEST: MAX-NORM TOO LARGE ({:.2e} > tol={:.2e})".format(
                    maxnorm_val, float(self.specs["max_norm_tol"])
                )

        if reason:
            self.setStatus(self.fail, reason)

        return output
