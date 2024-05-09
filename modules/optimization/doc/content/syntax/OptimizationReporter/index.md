# OptimizationReporter

!syntax description /OptimizationReporter/OptimizationReporter

## Overview

The `OptimizationReporter` sets up a reporter used for communicating data between the optimization executioner and the transfers to the sub-app forward and adjoint solves.  Only one `OptimizationReporter` is allowed per main-app.

## Example Input File Syntax

There is one required parameter in the `OptimizationReporter` specified in
[optRep] used to define the design parameters.
[!param](/OptimizationReporter/GeneralOptimization/parameter_names), defines a
vector of names for each group of design parameters.  Depending on the which
`OptimizationReporter`  used, the number of values per parameter will need to be defined
by [!param](/OptimizationReporter/GeneralOptimization/num_values) or by a reporter [!param](/OptimizationReporter/GeneralOptimization/num_values_name) that holds those
values. Both
of these will be a vector
containing the number of design parameters in each group named in
`parameter_names`.  Optional parameters bounds can be placed on each parameter
group using [!param](/OptimizationReporter/GeneralOptimization/lower_bounds) and
[!param](/OptimizationReporter/GeneralOptimization/upper_bounds).

!listing test/tests/optimizationreporter/point_loads/main.i
         block=OptimizationReporter
         id=optRep


!syntax list /OptimizationReporter objects=True actions=False subsystems=False groups=OptimizationApp

!syntax list /OptimizationReporter objects=False actions=True subsystems=False
