# StatisticsReporter

!syntax description /Reporters/StatisticsReporter

## Description

The `StatisticsReporter` object computes statistical values for each vector of
`VectorPostprocessor` (VPP) objects or support values from Reporters.  The results are output in
values with names based on the input data and the desired statistic calculation. Optionally
confidence level intervals can be computed.

## Statistics

The statistics to compute are indicated by the [!param](/Reporters/StatisticsReporter/compute)
parameter, which can contain multiple values as listed below. Note that multiple
statistical measures can be computed simultaneously by passing in more than one to the input
parameter.  The current statistical measures the `StatisticsReporter` can compute are:

- +minimum+

  `compute = min`\\
  Computes the minimum value for the supplied vectors.

- +maximum+

  `compute = max`\\
  Computes the maximum value for the supplied vectors.

- +sum+

  `compute = sum`\\
  Computes the sum ($\Sigma$) of the supplied vectors $\vec{v}$, where $N$ is the length of the vector:

  !equation
  \Sigma = \sum_{i=1}^N{v_i}

- +mean+

  `compute = average`\\
  Computes the average ($\bar{v}$) of the supplied vectors $\vec{v}$:

  !equation
  \bar{v} = \frac{\sum_{i=1}^{N}{v_i}}{N}

- +standard deviation+

  `compute = stddev`\\
  Computes the standard deviation ($\sigma$) of the supplied vectors $\vec{v}$:

  !equation
  \sigma = \sqrt{\frac{\sum_{i=1}^{N}{(v_i - \bar{v})^2}}{N-1}}

- +L2-Norm+

  `compute = norm2`\\
  Computes the L2-norm, $|v|_2$ of the supplied vectors $\vec{v}$, this is also known as the
  Euclidean Norm or the "distance":

  !equation
  |v|_2 = \sqrt{\sum_{i=1}^{N}{{v_i}^2}}

- +standard error+

  `compute = stderr`\\
  Computes the standard error ($\sigma_{\bar{v}}$) of the supplied vectors $\vec{v}$:

  !equation
  \sigma_{\bar{v}} = \frac{\sigma}{\sqrt{N}}

- +ratio+

  `compute = ratio`\\
  Computes the ratio of the maximum to the minimu of the supplied data.

- +median+

  `compute = median`\\
  Computes the median of the supplied vectors $\vec{v} = [v_1,...,v_N]$:

  !equation
  \mathrm{median}(\vec{v}) = \left\{ \begin{array}{ll}
    w_{(N+1)/2}, & \text{if } N \text{ is odd} \\
    \frac{w_{N/2} + w_{N/2+1}}{2}, & \text{if } N \text{ is even} \\
    \end{array} \right .

  where

  !equation
  \vec{w} = \mathrm{sort}(\vec{v})



## Confidence Levels

Bootstrap confidence level intervals, as defined by [!cite](tibshirani1993introduction), are enabled
by specifying the desired levels using the
[!param](/Reporters/StatisticsReporter/ci_levels) parameter and setting
the method of calculation using the
[!param](/Reporters/StatisticsReporter/ci_method).
The levels listed should be in the range (0, 1). For example, the levels 0.05, 0.95 result in the
computation of the 0.05, 0.95 confidence level intervals.

Enabling the confidence level intervals will compute the intervals for each level and each statistic
and the result will appear in the same output vector as the associated statistic calculation.

The available methods include the following:

- +percentile+: Percentile bootstrap method as defined in Ch. 13 of [!cite](tibshirani1993introduction).

## Example 1: Statistics

The following input file snippet demonstrates how to compute various statistics using the
`StatisticsReporter` object.

!listing reporters/statistics/statistics.i block=Reporters

This block results in the following JSON file output.

!listing reporters/statistics/gold/statistics_out.json


## Example 2: Confidence Levels

The following input file snippet demonstrates how to compute various statistics and
confidence levels using the `StatisticsReporter` object.

!listing reporters/bootstrap_statistics/percentile/percentile.i block=Reporters

This block results in the following JSON file.

!listing reporters/bootstrap_statistics/percentile/gold/percentile_out.json

!syntax parameters /Reporters/StatisticsReporter

!syntax inputs /Reporters/StatisticsReporter

!syntax children /Reporters/StatisticsReporter
