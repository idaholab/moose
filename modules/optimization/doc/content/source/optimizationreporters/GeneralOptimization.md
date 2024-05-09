# GeneralOptimization

!syntax description /OptimizationReporter/GeneralOptimization

## Overview


`GeneralOptimization` is responsible for creating parameter vector(s) for
optimization, setting the parameter initial condition for the optimization, and setting
parameter bounds and constraints. Although the [Optimize.md] executioner holds a single vector
for parameter values, this vector can be split into groups of parameters. This
is done by specifying a name for each group with
[!param](/OptimizationReporter/GeneralOptimization/parameter_names) and the
number of parameters in each group with
[!param](/OptimizationReporter/GeneralOptimization/num_values) or [!param](/OptimizationReporter/GeneralOptimization/num_values_name). The total number
of parameters is ultimately defined by the sum of
[!param](/OptimizationReporter/GeneralOptimization/num_values) or the values
inside of the reporter defined by [!param](/OptimizationReporter/GeneralOptimization/num_values_name). The initial
condition for the optimization can then be defined with
[!param](/OptimizationReporter/GeneralOptimization/initial_condition), where a
vector of data must defined for each group.  This vector can be a single value in
which case all parameters in that group are set to that value or a value can be
set for every parameter in that group.  The lower and upper bounds for the
parameters can then specified by
[!param](/OptimizationReporter/GeneralOptimization/lower_bounds) and
[!param](/OptimizationReporter/GeneralOptimization/upper_bounds), respectively.
The bounds follow the same input format rules as the `initial_condition`.  If no
initial conditions are provided, the parameters are initialized with 0.  Default
values for `upper_bounds` and `lower_bounds` are `std::numeric<Real>::max()` and
`std::numeric<Real>::lower()`, respectively.  These bounds are only applied if a
bounded optimization algorithm is used. The reporter that holds the objective
value is defined by
[!param](/OptimizationReporter/GeneralOptimization/objective_name).
Additionally, equality and inequality constraints can be defined by
[!param](/OptimizationReporter/GeneralOptimization/equality_names) and
[!param](/OptimizationReporter/GeneralOptimization/inequality_names),
respectively.


!syntax parameters /OptimizationReporter/GeneralOptimization

!syntax inputs /OptimizationReporter/GeneralOptimization
