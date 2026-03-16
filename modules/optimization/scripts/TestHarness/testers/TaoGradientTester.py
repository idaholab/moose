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
            "max_rel_tol",
            1e-5,
            "Tolerance for the relative max-norm ||G - Gfd||/||G||. Should be near "
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
        params.addParam(
            "only_first_gradient",
            True,
            "Check only the first gradient comparison. If False, "
            "all gradient comparisons in the output are checked.",
        )
        params.addParam(
            "turn_off_exodus_output", True, "Whether to set exodus=false in Outputs"
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

        if specs["turn_off_exodus_output"]:
            self.specs["cli_args"][:0] = ["Outputs/exodus=false"]

        tao_solver = specs["tao_solver"]

        petsc_iname = (
            "-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_ls_type"
        )
        petsc_value = "1 true true false unit"

        if specs["tao_fd_delta"] is not None:
            petsc_iname += " -tao_fd_delta"
            petsc_value += " " + str(specs["tao_fd_delta"])

        self.specs["cli_args"][:0] = [
            "Executioner/tao_solver=" + tao_solver,
            "Executioner/petsc_options_iname='" + petsc_iname + "'",
            "Executioner/petsc_options_value='" + petsc_value + "'",
            "Executioner/petsc_options='-tao_test_gradient_view'",
            "Executioner/verbose=true",
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

        cosine_pattern = r"\(Gfd'G\)/\|\|Gfd\|\|\|\|G\|\|\s*=\s*(\S+)"
        maxnorm_pattern = r"max-norm\s+\|\|G\s*-\s*Gfd\|\|/\|\|G\|\|\s*=\s*([^,\s]+)"

        # Match all gradient test outputs in the output
        cosine_matches = list(re.finditer(cosine_pattern, runner_output))
        maxnorm_matches = list(re.finditer(maxnorm_pattern, runner_output))

        reason = (
            "EXPECTED OUTPUT for '(Gfd'G)/||Gfd||||G||' "
            "and 'max-norm ||G - Gfd||/||G||' NOT FOUND"
        )

        if cosine_matches and maxnorm_matches:
            # If only_first_gradient, check just the first; otherwise check all
            if str(self.specs["only_first_gradient"]).lower() == "true":
                pairs = [(cosine_matches[0], maxnorm_matches[0])]
            else:
                pairs = zip(cosine_matches, maxnorm_matches)

            reason = ""
            for cosine_match, maxnorm_match in pairs:
                cosine_val = self.__strToFloat(cosine_match.group(1))
                maxnorm_val = self.__strToFloat(maxnorm_match.group(1))

                cosine_err = abs(1.0 - cosine_val)
                reason = ""
                if cosine_err > float(self.specs["cosine_tol"]):
                    reason = "GRADIENT TEST: ANGLE COSINE {} IS NOT 1 (err={:.2e} > tol={:.2e})".format(
                        cosine_val, cosine_err, float(self.specs["cosine_tol"])
                    )
                if maxnorm_val > float(self.specs["max_rel_tol"]):
                    if reason:
                        reason += "; "
                    reason += "GRADIENT TEST: MAX-NORM TOO LARGE ({:.2e} > tol={:.2e})".format(
                        maxnorm_val, float(self.specs["max_rel_tol"])
                    )

                # Break on first failure
                if reason:
                    break

        if reason:
            self.setStatus(self.fail, reason)

        return output
