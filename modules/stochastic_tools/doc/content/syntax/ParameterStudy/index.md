# ParameterStudy

!syntax description /ParameterStudy/ParameterStudyAction

## Overview

`ParameterStudy` is meant to be a simplified syntax for running common parameter studies like the one described in [examples/parameter_study.md]. The idea with this syntax is users do not need to understand or go through the modular design of the stochastic tools module to run a basic parameter study. The [ParameterStudyAction.md] builds the necessary objects programmatically, objects including [distributions](Distributions/index.md), [samplers](Samplers/index.md), [multi-apps](MultiApps/index.md), and [transfers](Transfers/index.md). The following sections desribe in detail the capabilities of this system, but the basic functionalities include:

1. Defining parameters and quantities of interest (QoIs) within the given physics input.
1. Perturbing the parameters using the specified sampling method.
1. Efficiently running each instance of the perturbation.
1. Gathering the QoI results.
1. Computing statistics on the QoIs
1. Outputting the QoIs and statistics.

## Defining the Study

The problem containing the physics is defined via another input file and specified using the [!param](/ParameterStudy/input) parameter. The idea is that this input defines the "nominal" case of the problem; as such, it should be able to run independently (without the `ParameterStudy` block).

The parameters being perturbed in the study are specified using the [!param](/ParameterStudy/parameters) parameter. This is a list of parameters in the physics input that use [CommandLine.md] syntax.

The QoIs for the study are specified using the [!param](/ParameterStudy/quantities_of_interest) parameter. This is a list of [reporter](Reporters/index.md) values with `<object_name>/<value_name>` syntax. [Postprocessor](Postprocessors/index.md) values use `<pp_name>/value` syntax. [Vector-postprocessor](VectorPostprocessors/index.md) values use `<vpp_name>/<vector_name>` syntax. This parameter is optional as users may want to do their own analysis using outputs, such as CSV or Exodus, generated from the physics input.

Whether or not to computate statistics on the QoIs is defined by the [!param](/ParameterStudy/compute_statistics) parameter. The default for this parameter is `true`, so statistics will be computed by default if QoIs are specified with the [!param](/ParameterStudy/quantities_of_interest) parameter. A [StatisticsReporter.md] object is created, which by default computes the mean and standard deviation of the QoIs with 90%, 95%, and 99% confidence intervals. The type of statistics can be specified with [!param](/ParameterStudy/statistics) and the confidence interval computation can specified with [!param](/ParameterStudy/ci_levels) and [!param](/ParameterStudy/ci_replicates), see [StatisticsReporter.md] for more details on confidence interval computation.

## Sampling Method id=sec:sampling

There are various methods of sampling included with the `ParameterStudy` syntax, including random and pre-defined techniques described below. This method is specified using the [!param](/ParameterStudy/sampling_type) parameter.

### Random Sampling id=sec:random

The `monte-carlo` and `lhs` options in [!param](/ParameterStudy/sampling_type) implement random sampling using the [MonteCarloSampler.md] and [LatinHypercubeSampler.md] samplers, respectively. [!param](/ParameterStudy/num_samples) is a required parameter that defines the total number of samples. [!param](/ParameterStudy/distributions) is also a required parameter that defines the probablity distribution for each parameter. The available distributions are described in the following subsection. Each distribution type have their own set of required parameters that are specified via a list of values. For example, defining a random sampling with three parameters using `uniform`, `normal`, and `uniform` distributions is shown below:

!listing!
[ParameterStudy]
  input = sub.i
  parameters = 'p1 p2 p3'
  sampling_type = monte-carlo
  num_samples = 10
  distributions = 'uniform normal uniform'
  uniform_lower_bound = '1 100'
  uniform_upper_bound = '2 200'
  normal_mean = '0'
  normal_standard_deviation = '1' 
[]
!listing-end!

#### Uniform Distribution

The `uniform` option in [!param](/ParameterStudy/distributions) builds a [Uniform.md] distribution. The [!param](/ParameterStudy/uniform_lower_bound) and [!param](/ParameterStudy/uniform_upper_bound) parameters are required if one or more of these options are specified.

#### Normal Distribution

The `normal` option in [!param](/ParameterStudy/distributions) builds a [Normal.md] distribution. The [!param](/ParameterStudy/normal_mean) and [!param](/ParameterStudy/normal_standard_deviation) parameters are required if one or more of these options are specified.

#### Weibull Distribution

The `wiebull` option in [!param](/ParameterStudy/distributions) builds a [Weibull.md] distribution. The [!param](/ParameterStudy/weibull_location), [!param](/ParameterStudy/weibull_scale), and [!param](/ParameterStudy/weibull_shape) parameters are required if one or more of these options are specified.

#### Lognormal Distribution

The `lognormal` option in [!param](/ParameterStudy/distributions) builds a [Lognormal.md] distribution. The [!param](/ParameterStudy/lognormal_location) and [!param](/ParameterStudy/lognormal_scale) parameters are required if one or more of these options are specified.

#### Truncated Normal Distribution

The `tnormal` option in [!param](/ParameterStudy/distributions) builds a [TruncatedNormal.md] distribution. The [!param](/ParameterStudy/tnormal_mean), [!param](/ParameterStudy/tnormal_standard_deviation), [!param](/ParameterStudy/tnormal_lower_bound), and [!param](/ParameterStudy/tnormal_upper_bound) parameters are required if one or more of these options are specified.

### Cartesian Product Sampling

The `cartesian-product` option in [!param](/ParameterStudy/sampling_type) implements Cartesian product sampling using the [CartesianProduct](CartesianProductSampler.md) sampler. [!param](/ParameterStudy/linear_space_items) is a required parameter with this option.

### User-Defined Matrix Sampling

A pre-defined matrix for sampling can be implemented using the `csv` or `input-matrix` options in [!param](/ParameterStudy/sampling_type). 

`csv` builds a [CSVSampler.md] sampler with the CSV file being specified using the [!param](/ParameterStudy/csv_samples_file) parameter. The choice and order of the matrix columns can be specified using the [!param](/ParameterStudy/csv_column_indices) or [!param](/ParameterStudy/csv_column_names) parameters.

`input-matrix` builds a [InputMatrixSampler.md] sampler with the matrix being specified using the [!param](/ParameterStudy/input_matrix) parameter.

## Execution Mode

The stochastic tools module has a number of different ways to perform stochastic sampling, as described in [batch_mode.md]. The permissable and most efficient mode is largely based on the type of parameters being perturbed and the problem's physics. There are five different mode options using the [!param](/ParameterStudy/multiapp_mode) parameter, which are described in the following subsections.

Additionally, it is often useful to define the minimum processors to use when running the samples. Typically this is done for large models in batch mode to avoid excessive memory usage. The [!param](/ParameterStudy/min_procs_per_sample) will utilize this capability.

### Normal mode

The `normal` option for [!param](/ParameterStudy/multiapp_mode) runs the study in "normal" mode, which creates a sub-application for each sample. The sub-applications are created upfront and run sequentially. This mode is arguably the *least efficient* option as it can become extremely memory intensive. This option mainly exisits to replicate the typical execution mode for other MOOSE [MultiApps](MultiApps/index.md). It also serves as a useful debugging tool for stochastic tools module development.

### Batch-Reset Mode

The `batch-reset` option for [!param](/ParameterStudy/multiapp_mode) runs the study in "batch-reset" mode, which creates a sub-appliction per processor, or set of processors, and re-initializes it for each sample. This mode is the recommended mode for general problems where the parameters are not (or not known to be) controllable. The controllability of objects can be found in their documentation; for instance, BCs/DirichletBC/[!param](/BCs/DirichletBC/value) is controllable, but Mesh/GeneratedMeshGenerator/[!param](/Mesh/GeneratedMeshGenerator/xmax) is not controllable.

### Batch-Restore Mode

The `batch-restore` option for [!param](/ParameterStudy/multiapp_mode) runs the study in "batch-restore" mode. Similar to `batch-reset`, `batch-restore` creates a sub-application per processor, but does not re-initialize. Instead it "restores" the sub-application to its state before the solve took place. This mode can be more efficient than `batch-reset` since the application's initialization is skipped, reusing the mesh and system vectors already built. However, the parameters being perturbed +must+ be controllable. Parameters involved in mesh generation and initial conditions are usually not controllable.

### Batch-Restore Mode -- Keep Solution

The `batch-keep-solution` option for [!param](/ParameterStudy/multiapp_mode) runs the study in "batch-restore" mode. The difference with `batch-restore` is that the solution from the previous sample is reused as the initial guess. This can reduce the solve time for subsequent samples since the previous sample's solution is most likely a better initial guess than the default or user-defined one. This mode is +not+ recommended for transient physics since it will alter the initial condition, which will change the resulting solution. This mode is recommended for pseudo transients, where a steady-state solution is the result of the simulation.

### Batch-Restore Mode -- No Restore

The `batch-no-restore` option for [!param](/ParameterStudy/multiapp_mode) runs the study in "batch-restore" mode, except the sub-application is not restored. Instead, the sub-application's solve is simply repeated with the newly perturbed parameters. This mode provides another efficiency by skipping this restoration step. This mode +only+ works with steady-state problems like those run with the [Steady.md] and [Eigenvalue.md] executioners, as transient problems need to be restored to the original time step.

### Automatic Mode Detection

While the execution mode can be explicitly specifyied using the [!param](/ParameterStudy/multiapp_mode) parameter, if this parameter is unspecified, the action will attempt to run in the most efficient mode by reading the physics input file. First, it will determine if the perturbed [!param](/ParameterStudy/parameters) are all controllable. If not all the parameter are controllable, the action will use `batch-reset` mode. This detection is a bit rudimentary, so it might not detect all controllable parameters. Next, it will determine what type of execution is being performed. If the `Executioner/type` is [Steady.md] or [Eigenvalue.md], then it will use `batch-no-restore` mode. If the executioner is [Transient.md] with `steady_state_detection = true`, then it will assume the simulation is pseudo-transient and use `batch-keep-solution`. All other cases with controllable parameters will use `batch-restore`. This automatic detection is not meant to be exhaustive, so it is recommended that users fully understand the problem and use the [!param](/ParameterStudy/multiapp_mode) parameter.

## Outputs

The `ParameterStudy` syntax provides ways to output the results of the study in CSV and/or JSON format. The sampler matrix (the values of the perturbed parameters) and the resulting QoI values can be output by setting the [!param](/ParameterStudy/output_type) parameter to `csv` and/or `json`. However, the CSV output will not contain vector-type QoIs, like those from vector-postprocessors. The default for both of this parameter is `json`. A JSON output is automatically created if QoI statistics are computed.

## List of Objects

Typically performing parameter studies only requires the `ParameterStudy` block. However, `ParameterStudy` does not prevent the addition of more objects by using other valid input syntax. This section gives advanced users more information on how to make other syntax work with `ParameterStudy` seamlessly.

All the objects built with [ParameterStudyAction.md] can be shown on console using the [!param](/ParameterStudy/show_study_objects) parameter. [tab:study_objects] lists all the objects created.

!table id=tab:study_objects caption=Objects created using the `ParameterStudy` syntax
| Base Type | Type | Name |
| - | - | - |
| Action | [StochasticToolsAction.md] | `ParameterStudy_stochastic_tools_action` |
| [Distribution](Distributions/index.md) | See [#sec:random] | `study_distribution_<i>`$^a$ |
| [Sampler](Samplers/index.md) | See [#sec:sampling] | `study_sampler` |
| [MultiApp](MultiApps/index.md) | [SamplerFullSolveMultiApp.md] | `study_app` |
| [Control](Controls/index.md) | [MultiAppSamplerControl.md] | `study_multiapp_control`$^b$ |
| [Transfer](Transfers/index.md) | [SamplerParameterTransfer.md] | `study_parameter_transfer`$^c$ |
| [Transfer](Transfers/index.md) | [SamplerReporterTransfer.md] | `study_qoi_transfer`$^d$ |
| [Reporter](Reporters/index.md) | [StochasticMatrix.md] | `study_results`|
| [Reporter](Reporters/index.md) | [StatisticsReporter.md] | `study_statistics`$^{d,e}$ |
| [Output](syntax/Outputs/index.md) | [CSV.md] | `csv`$^f$ |
| [Output](syntax/Outputs/index.md) | [JSONOutput.md] | `json`$^g$ |

!style fontsize=small
$^a$ `<i>` corresponds to the index of the distribution in [!param](/ParameterStudy/distributions);\\
$^b$ If [!param](/ParameterStudy/multiapp_mode) is `normal` or `batch-reset`;\\
$^c$ If [!param](/ParameterStudy/multiapp_mode) is `batch-restore`, `batch-keep-solution`, or `batch-no-restore`;\\
$^d$ If [!param](/ParameterStudy/quantities_of_interest) is specified;\\
$^e$ If [!param](/ParameterStudy/compute_statistics) is `true`;\\
$^f$ If [!param](/ParameterStudy/output_type) contains `csv`;\\
$^g$ If [!param](/ParameterStudy/output_type) contains `json` or [!param](/ParameterStudy/compute_statistics) is `true`;

## Example Input Syntax

The following two inputs are equivalent, taken from [examples/parameter_study.md].

!listing examples/parameter_study/main.i

!listing examples/parameter_study/main_parameter_study.i

!syntax parameters /ParameterStudy

!syntax list /ParameterStudy objects=True actions=False subsystems=False

!syntax list /ParameterStudy objects=False actions=False subsystems=True

!syntax list /ParameterStudy objects=False actions=True subsystems=False
