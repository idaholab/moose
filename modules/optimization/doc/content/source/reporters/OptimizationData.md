# OptimizationData

!syntax description /Reporters/OptimizationData

## Overview

`OptimizationData` is a reporter typically used for storing measurement data in a inverse optimization simulation. This reporter creates the following vectors:

- `measurement_xcoord`
- `measurement_ycoord`
- `measurement_zcoord`
- `measurement_time`
- `measurement_values`
- `simulation_values`
- `misfit_values`

The `measurement_` vectors can be filled in using [!param](/Reporters/OptimizationData/measurement_values), [!param](/Reporters/OptimizationData/measurement_points), and [!param](/Reporters/OptimizationData/measurement_times) or with a CSV file using [!param](/OptimizationReporter/OptimizationReporter/measurement_file), [!param](/OptimizationReporter/OptimizationReporter/file_xcoord), [!param](/OptimizationReporter/OptimizationReporter/file_ycoord), [!param](/OptimizationReporter/OptimizationReporter/file_zcoord), [!param](/OptimizationReporter/OptimizationReporter/file_value), and [!param](/OptimizationReporter/OptimizationReporter/file_time). The `simulation_values` and `misfit_values` are calculated in this object if [!param](/Reporters/OptimizationData/variable) is specified and the measurement location and values are filled. See [OptimizationReporter.md] for more information on these input parameters and what the vectors mean.

For cases where more than one variable is being sampled, the contribution of each sampled variable to the measurement value are specified by weights in csv file input using [!param](/Reporters/OptimizationData/file_variable_weights).  These weights are then multiplied by each variable in the forward problem by specifing [!param](/Reporters/OptimizationData/variable_weight_names).  There must be one [!param](/Reporters/OptimizationData/variable_weight_names) per [!param](/Reporters/OptimizationData/variable) being sampled and their ordering must be the same.  A typical use case for this would be to specify [!param](/Reporters/OptimizationData/file_variable_weights) in the main optimization application input file to be read in along with the other parameters normally read from file.  Next, these reporter weights would be transferred to the forward and adjoint problem subapps.  In the forward subapp, [!param](/Reporters/OptimizationData/file_variable_weights) and [!param](/Reporters/OptimizationData/variable) would both be specified in the `OptimizationData` block and would weight the different variable contributions to the `simulation_values` and `misfit_values`.  In the adjoint subapp, the weights are transferred into a reporter that is used to scale the misfit applied by the [ReporterPointSource.md] using [!param](/DiracKernels/ReporterPointSource/weight_name).

## Example Input File Syntax

`OptimizationData` is typically used in the forward and homogeneous optimization sub-applications to sample the solution for `simulation_values` and calculating `misfit_values`. The measurement data can either be specified directly or (more commonly) transferred from the optimization main application, while the simulation and misfit values are transferred to the main application. It is also typically used in the adjoint/gradient sub-application to hold the misfit values to apply the [ReporterPointSource.md] Dirac kernel.

The following code blocks demonstrate this utilization of `OptimizationData`. The [OptimizationReporter.md] in the main application reads the measurement locations and values. A [MultiAppReporterTransfer.md] transfers the measurement data to the forward, adjoint/gradient, and homogeneous application within the `OptimizationData` reporter declared in those inputs. The forward and homogeneous applications then sample the specified vector specified in [!param](/Reporters/OptimizationData/variable). The main application then receives the simulation data using a [MultiAppReporterTransfer.md], which [OptimizationReporter.md] uses to calculate the misfit and transfer to the adjoint/gradient sub-application, which uses that data to apply the [ReporterPointSource.md] kernel.

!listing point_loads/main.i block=OptimizationReporter Transfers

!listing point_loads/forward.i block=Reporters

!listing point_loads/forward_homogeneous.i block=Reporters

!listing point_loads/adjoint.i block=Reporters DiracKernels

!syntax parameters /Reporters/OptimizationData

!syntax inputs /Reporters/OptimizationData

!syntax children /Reporters/OptimizationData
