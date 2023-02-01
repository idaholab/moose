# Parameter Study Syntax

!style halign=center
[https://mooseframework.inl.gov/syntax/ParameterStudy/index.html](https://mooseframework.inl.gov/syntax/ParameterStudy/index.html)

!---

# ParameterStudy

- Action that simplifies syntax for simple parameter studies
- No need to identify value names or run into common, annoying input errors.
- Programmatic creation of `Samplers`, `Distributions`, `Controls`, `Transfers`, `Reporters`, and `Outputs` objects.
- Automatic multiapp mode detection

!---

!row!
!col! width=48%
```
[StochasticTools]
[]

[Distributions]
  [gamma]
    type = Uniform
    lower_bound = 0.5
    upper_bound = 2.5
  []
  [q_0]
    type = Weibull
    location = -110
    scale = 20
    shape = 1
  []
  [T_0]
    type = Normal
    mean = 300
    standard_deviation = 45
  []
  [s]
    type = Normal
    mean = 100
    standard_deviation = 25
  []
[]

[Samplers]
  [hypercube]
    type = LatinHypercube
    num_rows = 5000
    distributions = 'gamma q_0 T_0 s'
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = hypercube
    input_files = 'diffusion.i'
    mode = batch-restore
  []
[]

[Transfers]
  [parameters]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = hypercube
    parameters = 'Materials/constant/prop_values
                  Kernels/source/value
                  BCs/right/value BCs/left/value'
    to_control = 'stochastic'
  []
  [results]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = hypercube
    stochastic_reporter = results
    from_reporter = 'T_avg/value q_left/value'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
  []
  [stats]
    type = StatisticsReporter
    reporters = 'results/results:T_avg:value
                 results/results:q_left:value'
    compute = 'mean stddev'
    ci_method = 'percentile'
    ci_levels = '0.05 0.95'
  []
[]

[Outputs]
  execute_on = 'FINAL'
  [out]
    type = JSON
  []
[]
```
!col-end!

!col! width=4%
!!
!col-end!

!col! width=48%
```
[ParameterStudy]
  input = diffusion.i
  parameters = 'Materials/constant/prop_values
                Kernels/source/value
                BCs/right/value BCs/left/value'
  quantities_of_interest = 'T_avg/value q_left/value'

  sampling_type = lhs
  num_samples = 5000
  distributions = 'uniform weibull normal normal'
  uniform_lower_bound = 0.5
  uniform_upper_bound = 2.5
  weibull_location = -110
  weibull_scale = 20
  weibull_shape = 1
  normal_mean = '300 100'
  normal_standard_deviation = '45 25'
[]
```
!col-end!
!row-end!
