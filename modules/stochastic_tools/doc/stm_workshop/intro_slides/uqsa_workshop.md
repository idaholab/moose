# Uncertainty Quantification and Sensitivity Analysis

!---

# Computing Statistical Moments

!style halign=center fontsize=150%
[StatisticsReporter](StatisticsReporter.html)

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

!style fontsize=60%
!listing reporters/statistics/statistics_main.i
         block=Reporters/stats
         style=width:400px

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

!style fontsize=60%
!listing bootstrap_statistics/percentile/percentile_main.i
         block=Reporters/stats
         style=width:400px
         link=False

!col-end!
!row-end!

!---

# Sobol Sensitivities

!style! halign=center fontsize=150%
[Sobol](SobolSampler.md)

[SobolReporter](SobolReporter.md)
!style-end!

!row!
!col! width=70%
- Sobol indices are a global sensitivity analysis technique that can determine influence of parameter uncertainty on QoI uncertainty.
- The [SobolReporter](SobolReporter.md) object in conjunction with the `Sobol` sampler computes first-order, second-order, and total Sobol indices.

  - First-order and total indices indicate QoI sensitivity to a specific parameter
  - Second-order indices indicate non-linearities between two parameters

- The `Sobol` sampler requires the input of two other samplers to perform the sampling.
- Number of samples can be quite large with a large parameter space:

  !equation
  N = 2 N_a (M + 1)
!col-end!

!col! width=30%

!style fontsize=60%
!listing reporters/sobol/sobol_main.i
         block=Samplers Reporters/sobol
         style=width:400px;height:400px

!col-end!
!row-end!

!---

# Morris Filtering

!style! halign=center fontsize=150%
[MorrisSampler](MorrisSampler.md)

[MorrisReporter](MorrisReporter.md)
!style-end!

!row!
!col! width=70%
!style! fontsize=90%

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
!style-end!
!col-end!

!col! width=30%

!style fontsize=60%
!listing reporters/morris/morris_main.i
         block=Samplers Reporters/morris
         style=width:400px;height:400px

!col-end!
!row-end!

!---

# Workshop Statistics

!row!

!col! width=35%

!style fontsize=60%
!listing examples/workshop/step03.i
         diff=examples/workshop/step02.i
         style=width:400px

!col-end!

!col width=5%
!!

!col! width=60%

!style fontsize=50%
!listing! language=json
"stats": {
    "sampling_matrix_results:T_avg:value_MEAN": [
        292.04525124073933,
        [
            291.05955750899653,
            293.03675897580734
        ]
    ],
    "sampling_matrix_results:T_avg:value_STDDEV": [
        42.48125197567408,
        [
            41.772628701872996,
            43.178807046612356
        ]
    ],
    "sampling_matrix_results:q_left:value_MEAN": [
        7.717565253594722,
        [
            7.105391070784749,
            8.33016456253632
        ]
    ],
    "sampling_matrix_results:q_left:value_STDDEV": [
        26.25982391894163,
        [
            25.552483239209632,
            26.9620435963083
        ]
    ]
},
!listing-end!

!style fontsize=90%
| QoI              | Mean          | Standard Deviation |
|:-----------------|:--------------|:-------------------|
| $T_{avg}$        | 292           | 42.48              |
| (5.0%, 95.0%) CI | (291.1, 293)  | (41.77, 43.18)     |
| $q_{left}$       | 7.718         | 26.26              |
| (5.0%, 95.0%) CI | (7.105, 8.33) | (25.55, 26.96)     |

!col-end!

!row-end!

!---
