# Sensitivity Analysis

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
- The [!param](/Reporters/StatisticsReporter/percentile) option (recommended) utilizes a bootstrap method:

  - Shuffle the QoI vector a number of times to have that many new QoI vectors (shuffling causes repeated values).
  - Recompute the statistics on each QoI vector to produce a vector of statistics.
  - Sort the vector of statistics.
  - Select the value in the vector that is greater than the requested percent of values.

- [!param](/Reporters/StatisticsReporter/ci_replicates) specifies the number of shuffles.
- [!param](/Reporters/StatisticsReporter/ci_levels) specifies the percentile extracted (1%, 5%, 10%, 90%, 95%, and 99% are typical values).
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

- Morris filtering is another global sensitivity technique.
- Can require less computational effort than Sobol:

  !equation
  N = \texttt{trajectories}\times (M + 1)

- `MorrisReporter` computes the $\mu^{*}$ and $\sigma$ for each parameter:

  1. $\mu^{*} \approx 0, \sigma \approx 0$: no influential impact on the QoI.
  1. $\mu^{*} >> 0$: significant impact on the QoI.
  1. $\sigma >> 0$: nonlinear or interactive effects.
  1. $\mu^{*} >> 0, \sigma \approx 0$: additive or linear
  1. $\mu^{*} \approx 0, \sigma >> 0$: negligible aggregate effect on the QoI while nonlinear perturbations (perturbing in more than one direction) can be significant.
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

# Workshop Sobol Statistics

!row!

!col! width=35%

!style fontsize=60%
!listing examples/workshop/step04.i
         diff=examples/workshop/step02.i
         style=width:400px;height:300px

!col-end!

!col width=5%
!!

!col! width=60%

!style fontsize=50%
!listing! language=json
"sobol": {
    "sampling_matrix_results:T_avg:value": {
        "FIRST_ORDER": [
            [
                0.012886805005992073,
                -0.005371699727822437,
                0.94356770546739,
                0.03229000337826293
            ],
            [
                [
                    -0.05868786206191532,
                    -0.09728839898174543,
                    0.7882128660752169,
                    -0.06928830708176942
                ],
                [
                    0.08757250800355308,
                    0.07145037427674986,
                    1.0923493235473365,
                    0.14496698804687763
                ]
            ]
        ],
!listing-end!

!col-end!

!row-end!

!style fontsize=80%
| $S_i$ (5.0%, 95.0%) CI   | $D$                 | $q$                 | $T_0$           | $q_0$             |
|-------------------------:|:--------------------|:--------------------|:----------------|:------------------|
| $T_{avg}$                | 0.01289             | -0.005372           | 0.9436          | 0.03229           |
|                          | (-0.05869, 0.08757) | (-0.09729, 0.07145) | (0.7882, 1.092) | (-0.06929, 0.145) |
| $q_{left}$               | 0.07813             | 0.3364              | 0.1573          | 0.2562            |
|                          | (0.02931, 0.1329)   | (0.2908, 0.3849)    | (0.1229, 0.195) | (0.2183, 0.2961)  |

