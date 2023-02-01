# Uncertainty Quantification and Sensitivity Analysis

!---

# Computing Statistical Moments

!style halign=center
[https://mooseframework.inl.gov/source/reporters/StatisticsReporter.html](https://mooseframework.inl.gov/source/reporters/StatisticsReporter.html)

- STM is able to compute statistics on the resulting +distributed+ data.

!row!
!col! width=70%
- Mean

  !equation
  \bar{y} = \sum_{i=1}^N \frac{y_i}{N}

- Standard deviation

  !equation
  \sigma = \sqrt{\frac{\sum_{i=1}^{N}{(y_i - \bar{y})^2}}{N-1}}

- Median

  !equation
  \mathrm{median}(\vec{y}) = \left\{ \begin{array}{ll}
    w_{(N+1)/2}, & \text{if } N \text{ is odd} \\
    \frac{w_{N/2} + w_{N/2+1}}{2}, & \text{if } N \text{ is even} \\
    \end{array} \right .

  !equation
  \vec{w} = \mathrm{sort}(\vec{y})
!col-end!

!col! width=30%
```
[Reporters]
  [stats]
    type = StatisticsReporter
    reporters = 'storage/data:avg_u:value'
    compute = 'mean stddev median'
  []
[]
```
!col-end!
!row-end!

!---

# Confidence Intervals

!row!
!col! width=70%
- Confidence intervals are a way to determining how well the computed statistic is known.
- The `percentile` option (recommended) utilizes a bootstrap method:

  - Shuffle the QoI vector a number of times to have that many new QoI vectors (shuffling causes repeated values).
  - Recompute the statistics on each QoI vector to produce a vector of statistics.
  - Sort the vector of statistics.
  - Select the value in the vector that is greater than the requested percent of values.

- `ci_replicates` specifies the number of shuffles.
- `ci_levels` specifies the percentile extracted (1%, 5%, 10%, 90%, 95%, and 99% are typical values).
!col-end!

!col! width=30%
```
[Reporters]
  [stats]
    type = StatisticsReporter
    reporters = 'storage/data:avg_u:value'
    compute = 'mean stddev'
    ci_method = percentile
    ci_replicates = 10000
    ci_levels = '0.05 0.95'
  []
[]
```
!col-end!
!row-end!

!---

# Sobol Sensitivities

!style halign=center
[https://mooseframework.inl.gov/source/reporters/SobolReporter.html](https://mooseframework.inl.gov/source/reporters/SobolReporter.html)

!row!
!col! width=70%
- Sobol indices are a global sensitivity analysis technique that can determine influence of parameter uncertainty on QoI uncertainty.
- The `SobolReporter` object in conjunction with the `Sobol` sampler computes first-order, second-order, and total Sobol indices.

  - First-order and total indices indicate QoI sensitivity to a specific parameter
  - Second-order indices indicate non-linearities between two parameters

- The `Sobol` sampler requires the input of two other samplers to perform the sampling.
- Number of samples can be quite large with a large parameter space:

  !equation
  N = 2 N_a (M + 1)
!col-end!

!col! width=30%
```
[Samplers]
  [sample]
    type = MonteCarlo
    distributions = 'uniform normal'
    num_rows = 1024
    seed = 0
  []
  [resample]
    type = MonteCarlo
    distributions = 'uniform normal'
    num_rows = 1024
    seed = 1
  []
  [sobol]
    type = Sobol
    sampler_a = sample
    sampler_b = resample
  []
[]

[Reporters]
  [sobol]
    type = SobolReporter
    sampler = sobol
    reporters = 'storage/data:avg_u:value'
    ci_levels = '0.05 0.95'
    ci_replicates = 1000
  []
[]
```
!col-end!
!row-end!

!---

# Morris Filtering

!style halign=center
[https://mooseframework.inl.gov/source/reporters/MorrisReporter.html](https://mooseframework.inl.gov/source/reporters/MorrisReporter.html)

!row!
!col! width=70%
- Morris filtering is another global sensitivity analysis technique.
- Can be much less intensive than Sobol:

  !equation
  N = \texttt{trajectories}\times (M + 1)

- `MorrisReporter` computes the $\mu^{*}$ and $\sigma$ for each parameter:

  1. $\mu^{*} \approx 0, \sigma \approx 0$: parameter has no influential impact on the QoI.
  1. $\mu^{*} >> 0$: parameter has a significant impact on the QoI.
  1. $\sigma >> 0$: parameter has nonlinear or interactive effects.
  1. $\mu^{*} >> 0, \sigma \approx 0$: parameter is additive or linear
  1. $\mu^{*} \approx 0, \sigma >> 0$: parameter has a negligible aggregate effect on the QoI while nonlinear perturbations (perturbing in more than one direction) can be significant.
!col-end!

!col! width=30%
```
[Samplers]
  [morris]
    type = MorrisSampler
    distributions = 'uniform normal'
    trajectories = 10
    levels = 4
  []
[]

[Reporters]
  [morris]
    type = MorrisReporter
    reporters = 'storage/data:avg_u:value'
    ci_levels = '0.05 0.95'
    ci_replicates = 1000
  []
[]
```
!col-end!
!row-end!
