import math

import pandas as pd

from TestHarness.validation import ValidationCase


class TestCase(ValidationCase):
    @staticmethod
    def validParams():
        params = ValidationCase.validParams()
        params.addRequiredParam("validation_csv", "The controlled-case CSV output")
        params.addRequiredParam("validation_end_time", "The required final time")
        params.addRequiredParam("validation_dt", "The required fixed time-step size")
        params.addRequiredParam(
            "validation_max_nonlinear_iterations",
            "The benchmark solver-work budget for nonlinear iterations",
        )
        params.addRequiredParam(
            "validation_max_residual_evaluations",
            "The benchmark solver-work budget for residual evaluations",
        )
        params.addRequiredParam(
            "validation_max_linear_iterations",
            "The benchmark diagnostic linear-iteration budget",
        )
        params.addParam(
            "validation_comparison_csv",
            "An optional physically equivalent formulation output to compare",
        )
        return params

    def initialize(self):
        self.dataframe = pd.read_csv(self.getParam("validation_csv"))
        self.final = self.dataframe.iloc[-1]

    def testCompletionAndFiniteValues(self):
        end_time = float(self.getParam("validation_end_time"))
        self.addScalarData(
            "final_time",
            float(self.final["time"]),
            "Completed simulation time",
            None,
            bounds=(end_time, end_time),
        )
        finite = all(
            math.isfinite(float(value))
            for value in self.dataframe.select_dtypes(include="number").to_numpy().flat
        )
        self.addScalarData(
            "finite_values", int(finite), "All CSV metrics are finite", None, bounds=(1, 1)
        )

    def testNoCutbacks(self):
        expected_dt = float(self.getParam("validation_dt"))
        time_steps = self.dataframe[self.dataframe["time"] > 0]["timestep_size"]
        self.addScalarData(
            "minimum_timestep_size",
            float(time_steps.min()),
            "Minimum accepted time-step size",
            None,
            bounds=(expected_dt, expected_dt),
        )
        self.addScalarData(
            "maximum_timestep_size",
            float(time_steps.max()),
            "Maximum accepted time-step size",
            None,
            bounds=(expected_dt, expected_dt),
        )

    def testPhysicalResiduals(self):
        converged = self.dataframe[self.dataframe["time"] > 0]
        for column in ("kkt_error", "disp_x_residual", "disp_y_residual"):
            self.addScalarData(
                f"maximum_{column}",
                float(converged[column].max()),
                f"Maximum converged {column}",
                None,
                bounds=(0, 1e-10),
            )

    def testSolverWork(self):
        budgets = {
            "total_nonlinear_iterations": "validation_max_nonlinear_iterations",
            "num_residual_evaluations": "validation_max_residual_evaluations",
            "total_linear_iterations": "validation_max_linear_iterations",
        }
        for column, parameter in budgets.items():
            self.addScalarData(
                column,
                float(self.final[column]),
                column.replace("_", " ").capitalize(),
                None,
                bounds=(0, int(self.getParam(parameter))),
            )

    def testPhysicalAgreement(self):
        if not self.isParamValid("validation_comparison_csv"):
            self.addResult(
                self.Status.SKIP,
                "No comparison formulation was requested",
                validation=False,
            )
            return

        comparison = pd.read_csv(self.getParam("validation_comparison_csv")).iloc[-1]
        difference = max(
            abs(float(self.final[column]) - float(comparison[column]))
            for column in ("normal_pressure", "tangential_pressure")
        )
        self.addScalarData(
            "formulation_physical_difference",
            difference,
            "Maximum AC/HSW final physical-pressure difference",
            None,
            bounds=(0, 1e-8),
        )
