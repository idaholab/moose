# OptimizationReporter

!syntax description /OptimizationReporter/OptimizationReporter

## Overview

This optimization reporter provides a basic interface for inverse optimization with measurement data where all data is provided in the input file or from csv file. The objective function is defined as:

!equation
f(\mathbf{u}, \mathbf{p}) = \frac{1}{2}\sum^N_i \left(u_i - \tilde u_i\right)^2,

where \mathbf{u} represents the simulation solution and $u_i$ is the solution value at measurement location $i$. $\tilde u_i$ is the measurement value at location $i$. $\mathbf{p}$ is the vector of parameters being optimized that the simulation solution depends on.

## Measurement Data id=sec:measure_data

The measurement locations and values are defined either by a CSV file or are specified at input.

The CSV file is specified with the [!param](/OptimizationReporter/OptimizationReporter/measurement_file). This file must be at least four columns: x-coordinates indicated by [!param](/OptimizationReporter/OptimizationReporter/file_xcoord), y-coordinates indicated by [!param](/OptimizationReporter/OptimizationReporter/file_ycoord), z-coordinates inidicated by [!param](/OptimizationReporter/OptimizationReporter/file_zcoord), and values indicated by [!param](/OptimizationReporter/OptimizationReporter/file_value). A column for time coordinates can also be included and indicated by [!param](/OptimizationReporter/OptimizationReporter/file_time).

Additionally, locations and values can be specified at input using [!param](/OptimizationReporter/OptimizationReporter/measurement_points) for x-y-z coordinates, [!param](/OptimizationReporter/OptimizationReporter/measurement_times) for time coordinates, and [!param](/OptimizationReporter/OptimizationReporter/measurement_values) for the values.

## Optimization Parameters

`OptimizationReporter` is also responsible for creating parameter vector(s) for optimization, setting the initial condition for the optimization, and setting parameter bounds. Although the [Optimize.md] executioner holds a single vector for parameter values, this vector can be split into groups of parameters. This is done by specifying a name for each group with [!param](/OptimizationReporter/OptimizationReporter/parameter_names) and the number of parameters in each group with [!param](/OptimizationReporter/OptimizationReporter/num_values). The total number of parameters is ultimately defined by the sum of [!param](/OptimizationReporter/OptimizationReporter/num_values). The initial condition for the optimization can then be defined with [!param](/OptimizationReporter/OptimizationReporter/initial_condition), where a vector of data must defined for each group.  This vector an be a single value in which case all parameters in that group are set to that value or a value can be set for every parameter in that group.  The lower and upper bounds for the parameters can then specified by [!param](/OptimizationReporter/OptimizationReporter/lower_bounds) and [!param](/OptimizationReporter/OptimizationReporter/upper_bounds), respectively. The bounds follow the same input format rules as the `initial_condtion`.  If no initial conditions are provided, the parameters are initialized with 0.  Default values for `upper_bounds` and `lower_bounds` are std::numeric<Real>::max() and std::numeric<Real>::lower(), respectively.  These bounds are only applied if a bounded optimization algorithm is used.

## Declared Data

`OptimizationReporter` declares a number of vector reporter values that can be read by other objects and/or transferred to sub-applications with [MultiAppReporterTransfer.md]. [tab:or_vectors] lists each of these vectors. These vectors can be supplied or transferred by specifying `OptimizationReporter/<Vector Name>` at input.

!table id=tab:or_vectors caption=List of vectors declared by OptimizationReporter. $N$ is the number of measurement points.
| Description | Vector Name | Size |
| :- | - | - |
| $x$-coordinate | `measurement_xcoord` | $N$ |
| $y$-coordinate | `measurement_ycoord` | $N$ |
| $z$-coordinate | `measurement_zcoord` | $N$ |
| Time coordinate | `measurement_time` | $N$ |
| Measured values | `measurement_values` | $N$ |
| Simulation values | `simulation_values` | $N$ |
| $u_i - \tilde{u}_i$ | `misfit_values` | $N$ |
| Values of parameter group $g$ | [!param](/OptimizationReporter/OptimizationReporter/parameter_names)$_g$ | [!param](/OptimizationReporter/OptimizationReporter/num_values)$_g$ |
| Parameter Gradient | `adjoint` | $\sum_{g}$[!param](/OptimizationReporter/OptimizationReporter/num_values)$_g$ |


## Example Input Syntax

The following example creates three groups of parameters named `p1`, `p2`, and `p3` with two, four, and six parameters, respectively. There are several ways to define the initial conditions and upper and lower bounds.  Starting with the `initial_condition`, all parameters are specified for the `p1` parameter group, only one parameter is defined for the `p2` group making all parameter values in this group set to this value, and all parameters are defined for the `p3` group.  A similar mix of defining one or all of the lower and upper bounds are demonstrated in this input file.

It sets an initial condition and upper and lower bounds for every parameter. The measurement data is taken from a CSV file where the x-coordinate, y-coordinate, z-coordinate, and measurement value columns are specified with the header names of the file.

!listing base/optRep_fromCsv_mixBounds.i block=OptimizationReporter

!listing base/measurementData.csv

The measurement data can also be given directly, like in the following example.

!listing point_loads/main.i block=OptimizationReporter

!syntax parameters /OptimizationReporter/OptimizationReporter

!syntax inputs /OptimizationReporter/OptimizationReporter

!syntax children /OptimizationReporter/OptimizationReporter
