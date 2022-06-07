# AdaptiveImportanceStats

!syntax description /Reporters/AdaptiveImportanceStats

## Description

This Reporter class computes the statistics associated with the [AdaptiveImportanceSampler](AdaptiveImportanceSampler.md).

## Available statistics

The following statistics have been implemented:

- +Failure probability (pf)+

  This is the number of samples causing model failure divided by the total number evaluation
  samples. For computing this statistic, the samples from the training phase of [AdaptiveImportanceSampler](AdaptiveImportanceSampler.md)
  are not used. The equation for computing pf is given by [!cite](au1999new):

  !equation id=eqn:ais_stats1
  \hat{P}_f^{\textrm{AIS}} = \frac{1}{N}~\sum_{i=1}^N I\big(F_i(\pmb{x})\geq \mathcal{F}\big)~\frac{q(\pmb{x})}{f(\pmb{x})}

  where, $q(.)$ is the nominal density, $f(.)$ is the importance density, and $I[.]$ is
  the indicator function of model failure.

- +Coefficient of variation (cov)+

  This quantity is computed as the ratio of square root of variance of pf to the
  mean pf. The variance of pf is given by [!cite](au1999new):

  !equation id=eqn:ais_stats2
  \textrm{Var}(\hat{P}_f^{\textrm{AIS}}) = \frac{1}{N}~\Bigg\{\frac{1}{N}~\sum_{i=1}^{N}\Bigg[I\big(F_i(\pmb{x})\geq \mathcal{F}\big)~\frac{q(\pmb{x})}{f(\pmb{x})}\Bigg]^2-\big(\hat{P}_f^{\textrm{AIS}}\big)^2\Bigg\}

  The cov is given by:

  !equation id=eqn:ais_stats3
  \hat{\delta}^{\textrm{AIS}} = \frac{\sqrt{\textrm{Var}(\hat{P}_f^{\textrm{AIS}})}}{\hat{P}_f^{\textrm{AIS}}}
  \end{equation}

  For computing this statistic, the samples from the training phase of [AdaptiveImportanceSampler](AdaptiveImportanceSampler.md)
  are not used.

- +Means of the importance distributions (mu_imp)+

  These are the means of the importance distributions in a standard Normal space.

- +Standard deviations of the importance distributions (std_imp)+

  These are the standard deviations of the importance distributions in a standard Normal space.

!syntax parameters /Reporters/AdaptiveImportanceStats

!syntax inputs /Reporters/AdaptiveImportanceStats

!syntax children /Reporters/AdaptiveImportanceStats
