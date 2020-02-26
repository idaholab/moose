# Statistics

!syntax description /VectorPostprocessors/Statistics

## Description

The `Statistics` object computes statistical values for each vector of other
`VectorPostprocessor` (VPP) objects.  The results are output in vectors that are assigned names
based on the VPP and vector name (object-vector) and each entry in the vector corresponding
the the desired statistics and optionally confidence level intervals.

The first column, named `stat_type` and contains an unique integer identifier for the type of
statistical measure computed and the confidence levels, if computed.

## Statistics

The statistics to compute are indicated by the
[!param](/VectorPostprocessors/Statistics/compute) parameter, which can contain
multiple values as listed below. This list also includes the associated numeric identifier
that is included in the `stat_type` vector output the VPP object.

measures are chosen using the `stats` input parameter.  Note that multiple
statistical measures can be computed simultaneously by passing in more than one to the `stats` input
parameter.  The current statistical measures (and their `stat_type` identifier) the
`StatisticsPostprocessor` can compute are:

- +minimum (0)+

  `compute = min`\\
  Computes the minimum value for the supplied vectors.

- +maximum (1)+

  `compute = max`\\
  Computes the maximum value for the supplied vectors.

- +sum (2)+

  `compute = sum`\\
  Computes the sum ($\Sigma$) of the supplied vectors $\vec{v}$, where $N$ is the length of the vector:

  !equation
  \Sigma = \sum_{i=1}^N{v_i}

- +mean (3)+

  `compute = average`\\
  Computes the average ($\bar{v}$) of the supplied vectors $\vec{v}$:

  !equation
  \bar{v} = \frac{\sum_{i=1}^{N}{v_i}}{N}

- +standard deviation (4)+

  `compute = stddev`\\
  Computes the standard deviation ($\sigma$) of the supplied vectors $\vec{v}$:

  !equation
  \sigma = \sqrt{\frac{\sum_{i=1}^{N}{(v_i - \bar{v})^2}}{N-1}}

- +2-Norm (5)+

  `compute = norm2`\\
  Computes the 2-norm, $|v|_2$ of the supplied vectors $\vec{v}$, this is also known as the
  Euclidean Norm or the "distance":

  !equation
  |v|_2 = \sqrt{\sum_{i=1}^{N}{{v_i}^2}}

- +standard error (6)+

  `compute = stderr`\\
  Computes the standard error ($\sigma_{\bar{v}}$) of the supplied vectors $\vec{v}$:

  !equation
  \sigma_{\bar{v}} = \frac{\sigma}{\sqrt{N}}


## Confidence Levels

Bootstrap confidence level intervals, as defined by [!cite](tibshirani1993introduction), are enabled
by specifying the desired levels using the
[!param](/VectorPostprocessors/Statistics/ci_levels) parameter and setting
the method of calculation using the
[!param](/VectorPostprocessors/Statistics/ci_method).
The levels listed should be in the range (0, 0.5]. For example, the levels 0.05, 0.1, and 0.5 provided
result in the computation of the 0.05, 0.1, 0.5, 0.9, and 0.95 confidence level intervals.

Enabling the confidence level intervals will compute the intervals for each level and each statistic
and the result will appear in the same output vector as the associated statistic calculation. The
`stat_type` vector will include decimal values where the ones place indicates the statistic
computed and the decimal corresponds with the confidence level.

The available methods include the following:

- +percentile+: Percentile bootstrap method as defined in Ch. 13 of [!cite](tibshirani1993introduction).

## Example 1: Statistics

The following input file snippet demonstrates how to compute various statistics using the
`Statistics` object.

!listing statistics.i block=VectorPostprocessors

This block results in the following CSV file for the "stats" block of the input file. Notice
the first column corresponds with the numeric identifier for the statistics being computed.

!listing statistics/gold/statistics_out_stats_0001.csv


## Example 2: Confidence Levels

The following input file snippet demonstrates how to compute various statistics and
confidence levels using the `Statistics` object

!listing bootstrap_statistics/percentile/percentile.i block=VectorPostprocessors

This block results in the following CSV file for the "stats" block of the input file. Notice
the first column corresponds with the numeric identifier for the statistics being computed.

!listing bootstrap_statistics/percentile/gold/percentile_out_stats_0001.csv

!syntax parameters /VectorPostprocessors/Statistics

!syntax inputs /VectorPostprocessors/Statistics

!syntax children /VectorPostprocessors/Statistics

!bibtex bibliography
