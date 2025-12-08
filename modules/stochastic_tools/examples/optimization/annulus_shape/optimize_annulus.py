# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
import argparse
import numpy as np
from scipy.optimize import (
    shgo,
    differential_evolution,
    minimize,
    NonlinearConstraint,
    OptimizeResult,
)

from mooseutils import find_moose_executable_recursive
from moose_stochastic_tools import StochasticControl, StochasticRunOptions
from moose_stochastic_tools.StochasticControl import StochasticRunner


def cli_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()

    parser.add_argument("--input-file", "-i", type=str, default="annulus.i")
    parser.add_argument(
        "--executable",
        "-e",
        type=str,
        default=find_moose_executable_recursive(os.path.dirname(__file__)),
    )
    parser.add_argument("--num-procs", "-n", type=int, default=1)
    parser.add_argument("--volume", "-v", type=float, default=200.0)
    parser.add_argument("--mode", "-m", type=int, default=1)
    parser.add_argument("--optimizer", "-o", type=str, default="shgo")
    parser.add_argument("--cli-args", "-c", type=str, nargs="+", default=[])

    return parser.parse_args()


def computeTmax(runner: StochasticRunner, x: np.ndarray) -> np.ndarray | float:
    """Use the inputted runner to evaluate the inputted sample and
    retrieve the appropriate index for max temperature of the result.
    """
    y = runner(x)
    return y[:, 0] if y.ndim > 1 else y[0]


def computeVolume(runner: StochasticRunner, x: np.ndarray) -> np.ndarray | float:
    """Use the inputted runner to evaluate the inputted sample and
    retrieve the appropriate index for volume of the result.
    """
    y = runner(x)
    return y[:, 1] if y.ndim > 1 else y[1]


def optimize_annulus(
    input_file: str,
    executable: str,
    num_procs: int,
    volume: float,
    mode: int,
    optimizer: str,
    cli_args: list[str] = [],
) -> OptimizeResult:
    """Perform shape optimization of annular pipe.

    Parameters:
        input_file (str): Physics input file (options: 'annulus.i' and 'annulus_displaced_mesh.i')
        executable (str): Executable with stochastic-tools module included.
        num_procs (int): Number of processors to execute the optimization.
        volume (float): Volume of annulus to keep constant.
        mode (int): MultiApp mode of execution (See moose_stochastic_tools.StochasticRunOptions.MultiAppMode)
        optimizer (str): SciPy optimizer to use, e.g. 'shgo', 'dual_annealing', 'slsqp', etc.
        cli_args (list[str]): Extra command-line arguments for stochastic input.

    Returns
        OptimizeResults: Results of the optimization.
    """
    # Parameters to optimize depend on which input we're using
    if input_file.endswith("annulus.i"):
        params = ["inner_radius", "thickness"]
    elif input_file.endswith("annulus_displaced_mesh.i"):
        params = ["Postprocessors/inner_radius/value", "Postprocessors/thickness/value"]
    else:
        raise ValueError(f"Unknown input file {input_file}")

    # QoIs include the minimization function (max temperature) and constraint (volume)
    qois = ["Tmax/value", "volume/value"]

    # Build StochasticControl with options
    opts = StochasticRunOptions(
        num_procs=num_procs,
        multiapp_mode=StochasticRunOptions.MultiAppMode(mode),
        cli_args=cli_args,
    )
    with StochasticControl(executable, input_file, params, qois, opts) as runner:
        # This will ensure that if the same input is seen twice, it will only run the simulation once.
        runner.configCache()

        # Somewhat arbitrary bounds to the inner radius and thickness
        bounds = [(0.1, 10), (0.01, 10)]

        # Simplicial Homology Global Optimization
        if optimizer.lower() == "shgo":
            # Create lambdas so that 'runner' is not a required argument for the optimizer
            objective = lambda x: computeTmax(runner, x)
            eq_constraint = lambda x: computeVolume(runner, x) - volume

            result = shgo(
                objective,
                bounds=bounds,
                constraints=[{"type": "eq", "fun": eq_constraint}],
                workers=1 if num_procs == 1 else runner.parallelWorker,
            )

        # Differential evolution
        elif optimizer.lower() == "differential_evolution":
            # Differential evolution allows for "vectorization", sampling multiple rows at a time
            # The input it provides (and output expected) is actually transposed to what
            # the StochasticRunner provides.
            # Create lambdas so that 'runner' is not a required argument for the optimizer
            objective = lambda x: (
                computeTmax(runner, x.T) if x.ndim == 2 else computeTmax(runner, x)
            )
            eq_constraint = lambda x: (
                computeVolume(runner, x.T).reshape((1, -1))
                if x.ndim == 2
                else computeVolume(runner, x)
            )

            result = differential_evolution(
                objective,
                bounds=bounds,
                constraints=NonlinearConstraint(eq_constraint, volume, volume),
                vectorized=True,
                maxiter=10,
            )

        # Other methods, see scipy.optimize.minimize for details
        else:  # minimize-start
            # Create lambdas so that 'runner' is not a required argument for the optimizer
            objective = lambda x: computeTmax(runner, x)
            eq_constraint = lambda x: computeVolume(runner, x)

            result = minimize(
                objective,
                x0=np.array([6, 4]),
                method=optimizer.upper(),
                bounds=bounds,
                constraints=NonlinearConstraint(eq_constraint, volume, volume),
                options={"workers": 1 if num_procs == 1 else runner.parallelWorker},
            )

    return result


if __name__ == "__main__":

    args = cli_args()
    result = optimize_annulus(**vars(args))
    print(result)
