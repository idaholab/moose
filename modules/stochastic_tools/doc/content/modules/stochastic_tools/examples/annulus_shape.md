# Annulus Shape Optimization — SciPy + `StochasticControl`

This example demonstrates how to couple [SciPy optimization](https://docs.scipy.org/doc/scipy/reference/optimize.html) routines with [StochasticControl](StochasticControl.md) to minimize the maximum temperature in a 2D annular geometry while enforcing a constant volume constraint. It is a practical pattern for black‑box optimization where each function evaluation runs a MOOSE simulation and returns quantities of interest (QoIs).

## Problem Description

In this problem, we focus on an annular (ring-shaped) domain subjected to thermal conditions. The domain is defined by two concentric circles with an inner radius and thickness. The physical problem is heat conduction governed by the heat equation with a convective flux type boundary condition at the inner radius and an insulated outer radius. The convective heat transfer coefficient is dependent on the inner radius. A constant source term is distributed throughout the domain.

The optimization goal is to find the inner radius and thickness that minimize the maximum temperature within the domain while maintaining a fixed volume of the annulus. In other words, the design variables are the inner radius and thickness, and the objective function is the maximum temperature within the annulus. This is a constrained optimization problem because the volume of the annulus needs to remain constant during the optimization process.

This example problem is identical to the [shape optimization example](optimization/examples/shapeOpt_Annulus.md optional=True) in the [Optimization module](modules/optimization/index.md optional=True). The optimization module version utilizes gradients for its minimization solve, which makes it a faster method, but is more involved to formulate.

## Physics Inputs

There are two MOOSE inputs representing the physics. The first utilizes input variables for the inner radius and thickness to directly change the mesh that is generated. The other uses mesh displacement to change the domain of the ring.

### Mesh-Based Perturbation Input

The mesh-based input starts with definition of the input variables to-be optimized:

!listing annulus_shape/annulus.i end=[

The mesh is then generated based on these values:

!listing annulus_shape/annulus.i block=Mesh

The heat conduction problem is then defined:

!listing annulus_shape/annulus.i block=Variables Kernels BCs Materials Executioner

Finally, postprocessors are defined that represent the objective (`Tmax`) and constraint (`volume`):

!listing annulus_shape/annulus.i block=Postprocessors

### Mesh Displacement-Based Input

This input utilizes mesh-displacement to define the ring domain. This strategy is arguably more complex; however, it enables the use of STM's "batch-restore" functionality to prevent re-initialization of the sub-application and to re-use previous results as initial guesses.

Instead of input variables, the inner radius and thickness are defined as [ConstantPostprocessors](ConstantPostprocessor.md), whose [!param](/Postprocessors/ConstantPostprocessor/value) parameter is controllable:

!listing annulus_shape/annulus_displaced_mesh.i block=Postprocessors/inner_radius Postprocessors/thickness

The base mesh is fixed with an inner radius and thickness of 1:

!listing annulus_shape/annulus_displaced_mesh.i
    diff=annulus_shape/annulus.i
    block=Mesh

Displacement variables are defined, whose values depend on the postprocessors:

!listing annulus_shape/annulus_displaced_mesh.i block=GlobalParams AuxVariables AuxKernels

Note that the heat transfer coefficient also depends on the inner radius, so the convection boundary condition material needs adjustment:

!listing annulus_shape/annulus_displaced_mesh.i
    block=Postprocessors/h

!listing annulus_shape/annulus_displaced_mesh.i
    diff=annulus_shape/annulus.i
    block=Materials/convection

Finally, we have to ensure the rest of the heat conduction objects and postprocessors must use the displaced mesh:

!listing annulus_shape/annulus_displaced_mesh.i
    diff=annulus_shape/annulus.i
    block=Kernels BCs Postprocessors/Tmax Postprocessors/volume


## Optimization Using SciPy

The following presents an example python script that leverages various [SciPy optimizers](https://docs.scipy.org/doc/scipy/reference/optimize.html) to solve this optimization problem using [StochasticControl](StochasticControl.md) to drive the simulation. A description of the main function of this script is below:

!listing annulus_shape/optimize_annulus.py start=def optimize_annulus end=#

### Configuring the StochasticControl

The parameters to control are dependent on which input file is being utilized:
Next, the postprocessors defining the objective and constraints of the optimization are specified as QoIs of the simulation:
A `StochasticRunOptions` is then built, to be given as options for the `StochasticControl`.
The `StochasticControl` is created as a context manager which returns a `StochasticRunner` object.
Since the optimizers often run objective and constraint evaluation separately, but with the same sample, it is recommended to cache the input-output pairs using `configCache`. Finally, somewhat arbitrary bounds for the parameters are defined.

!listing annulus_shape/optimize_annulus.py start=# Parameters to optimize end=# Simplicial

### Helper functions (QoI adapters)

!listing annulus_shape/optimize_annulus.py start=def computeTmax end=def optimize_annulus

These wrap the runner and select the correct QoI column.

### Optimizer behaviors

#### 1) SHGO (global)

!listing annulus_shape/optimize_annulus.py start=if optimizer.lower() end=# Differential include-start=False

- Uses equality constraint via `constraints=[...]`.
- Sets `workers` to `runner.parallelWorker` when `num_procs > 1` to distribute evaluations.

#### 2) Differential Evolution (global, vectorized)

!listing annulus_shape/optimize_annulus.py start=elif optimizer.lower() end=# Other include-start=False

- Vectorized mode samples many candidates at once; SciPy passes a matrix shaped `(n_params, n_points)`. The code transposes to match `runner`’s `(n_points, n_params)` expectation.
- Equality constraint via `NonlinearConstraint`.
- `maxiter=10` for demonstration; increase for higher‑quality solutions.

#### 3) Local methods via `minimize`

!listing annulus_shape/optimize_annulus.py start=minimize-start end=return include-start=False

- Starts from `x0=[6, 4]`.
- Adds equality constraint with `NonlinearConstraint`.
- Parallel evaluation available through `runner.parallelWorker` in methods that respect the `workers` option.

## Running the Example

This example script can be run using the following command-line arguments:

```bash
python optimize_annulus.py <args>
```

!table
| Flag               | Default                | Description |
| :----------------- | ---------------------: | :---------- |
| `-i, --input-file` | `annulus.i`            | Use `annulus.i` or `annulus_displaced_mesh.i` |
| `-e, --executable` | `stochastic_tools-opt` | MOOSE executable with `stochastic_tools` |
| `-n, --num-procs`  | `1`                    | Number of processors for the stochastic run |
| `-v, --volume`     | `200.0`                | Equality constraint value passed to the optimizer |
| `-m, --mode`       | `1`                    | Integer mapped to `StochasticRunOptions.MultiAppMode` |
| `-o, --optimizer`  | `shgo`                 | One of `shgo`, `differential_evolution`, or any method supported by `scipy.optimize.minimize` (e.g., `SLSQP`, `COBYLA`, etc.) |

The resulting inner radius is approximately 1.198, the thickness is 6.877, and the max temperature is 160.3.
