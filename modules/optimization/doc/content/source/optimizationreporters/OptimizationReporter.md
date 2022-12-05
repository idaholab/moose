# OptimizationReporter

!syntax description /OptimizationReporter/OptimizationReporter

## Overview

This optimization reporter serves as the base class for inverse optimization with measurement data. The objective function is defined as:

!equation
f(\mathbf{u}, \mathbf{p}) = \frac{1}{2}\sum^N_i \left(u_i - \tilde u_i\right)^2,

where \mathbf{u} represents the simulation solution and $u_i$ is the solution value at measurement location $i$. $\tilde u_i$ is the measurement value at location $i$. $\mathbf{p}$ is the vector of parameters being optimized that the simulation solution depends on.

## Measurement Data id=sec:measure_data

The measurement locations and values are defined either by a CSV file or are specified at input. 

The CSV file is specified with the [!param](/OptimizationReporter/OptimizationReporter/measurement_file). This file must be at least four columns: x-coordinates indicated by [!param](/OptimizationReporter/OptimizationReporter/file_xcoord), y-coordinates indicated by [!param](/OptimizationReporter/OptimizationReporter/file_ycoord), z-coordinates inidicated by [!param](/OptimizationReporter/OptimizationReporter/file_zcoord), and values indicated by [!param](/OptimizationReporter/OptimizationReporter/file_value). A column for time coordinates can also be included and indicated by [!param](/OptimizationReporter/OptimizationReporter/file_time).

Additionally, locations and values can be specified at input using [!param](/OptimizationReporter/OptimizationReporter/measurement_points) for x-y-z coordinates, [!param](/OptimizationReporter/OptimizationReporter/measurement_times) for time coordinates, and [!param](/OptimizationReporter/OptimizationReporter/measurement_values) for the values.

## Optimization Parameters

`OptimizationReporter` is also responsible for creating parameter vector(s) for optimization, setting the initial condition for the optimization, and setting parameter bounds. Although the [Optimize.md] executioner holds a single vector for parameter values, this vector can be split into groups of parameters. This is done by specifying a name for each group with [!param](/OptimizationReporter/OptimizationReporter/parameter_names) and the number of parameters in each group with [!param](/OptimizationReporter/OptimizationReporter/num_values). The total number of parameters is ultimately defined by the sum of [!param](/OptimizationReporter/OptimizationReporter/num_values). The initial condition for the optimization can then be defined with [!param](/OptimizationReporter/OptimizationReporter/initial_condition), whose length needs to be same as the total number of parameters. The lower and upper bounds for the parameters can then specified by [!param](/OptimizationReporter/OptimizationReporter/lower_bounds) and [!param](/OptimizationReporter/OptimizationReporter/upper_bounds), respectively. If only one value is specified for each of these, that value is used for all the parameters, otherwise the length of these need to be the same as the total number of parameters.

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

The following example creates three groups of parameters named `p1`, `p2`, and `p3` with two, four, and six parameters, respectively. It sets an initial condition and upper and lower bounds for every parameter. The measurement data is taken from a CSV file where the x-coordinate, y-coordinate, z-coordinate, and measurement value columns are specified with the header names of the file.

!listing base/optRep_fromCsv.i block=OptimizationReporter

!listing base/measurementData.csv

The measurement data can also be given directly, like in the following example.

!listing point_loads/main.i block=OptimizationReporter

!syntax parameters /OptimizationReporter/OptimizationReporter

!syntax inputs /OptimizationReporter/OptimizationReporter

!syntax children /OptimizationReporter/OptimizationReporter
