# GeneralOptimization

!syntax description /OptimizationReporter/GeneralOptimization

## Overview


## Optimization Parameters

`GeneralOptimization` is also responsible for creating parameter vector(s) for
optimization, setting the initial condition for the optimization, and setting
parameter bounds and constraints. Although the [Optimize.md] executioner holds a single vector
for parameter values, this vector can be split into groups of parameters. This
is done by specifying a name for each group with
[!param](/OptimizationReporter/GeneralOptimization/parameter_names) and the
number of parameters in each group with
[!param](/OptimizationReporter/GeneralOptimization/num_values). The total number
of parameters is ultimately defined by the sum of
[!param](/OptimizationReporter/GeneralOptimization/num_values). The initial
condition for the optimization can then be defined with
[!param](/OptimizationReporter/GeneralOptimization/initial_condition), where a
vector of data must defined for each group.  This vector an be a single value in
which case all parameters in that group are set to that value or a value can be
set for every parameter in that group.  The lower and upper bounds for the
parameters can then specified by
[!param](/OptimizationReporter/GeneralOptimization/lower_bounds) and
[!param](/OptimizationReporter/GeneralOptimization/upper_bounds), respectively.
The bounds follow the same input format rules as the `initial_condtion`.  If no
initial conditions are provided, the parameters are initialized with 0.  Default
values for `upper_bounds` and `lower_bounds` are `std::numeric<Real>::max()` and
`std::numeric<Real>::lower()`, respectively.  These bounds are only applied if a
bounded optimization algorithm is used.

## Declared Data

`GeneralOptimization` declares a number of vector reporter values that can be read by other objects and/or transferred to sub-applications with [MultiAppReporterTransfer.md]. [tab:or_vectors] lists each of these vectors. These vectors can be supplied or transferred by specifying `GeneralOptimization/<Vector Name>` at input.

!table id=tab:or_vectors caption=List of vectors declared by GeneralOptimization. $N$ is the number of measurement points.
| Description | Vector Name | Size |
| :- | - | - |
| $x$-coordinate | `measurement_xcoord` | $N$ |
| $y$-coordinate | `measurement_ycoord` | $N$ |
| $z$-coordinate | `measurement_zcoord` | $N$ |
| Time coordinate | `measurement_time` | $N$ |
| Measured values | `measurement_values` | $N$ |
| Simulation values | `simulation_values` | $N$ |
| $u_i - \tilde{u}_i$ | `misfit_values` | $N$ |
| Values of parameter group $g$ | [!param](/OptimizationReporter/GeneralOptimization/parameter_names)$_g$ | [!param](/OptimizationReporter/GeneralOptimization/num_values)$_g$ |
| Parameter Gradient of parameter group $g$ | `grad_`[!param](/OptimizationReporter/GeneralOptimization/parameter_names)$_g$  | [!param](/OptimizationReporter/GeneralOptimization/num_values)$_g$ |

