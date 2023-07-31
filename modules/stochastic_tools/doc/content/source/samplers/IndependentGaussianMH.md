# IndependentGaussianMH (Independent Gaussian Metropolis-Hastings)

!syntax description /Samplers/IndependentGaussianMH

## Overview

The `proposeSamples` function from the [PMCMCBase](PMCMCBase.md) parent class is overriden. This proposal is a simple independent Metropolis-Hastings (M-H) using a Gaussian proposal centered on a seed state. The scale (standard deviation) of this Gaussian is user-specified for each model parameter. Note that the `IndependentGaussianMH` assumes that the noise variance (i.e., uncertainty due to model inadequacy plus experimental noise) is specified by the user. Expanding `IndependentGaussianMH` to infer the noise variance term will be considered in the future. If lower and upper bounds to the parameters are specified, the proposals are generated using a truncated Gaussian. Specifically, the new $p^{\text{th}}$ parallel proposal is given by:

\begin{equation}
\label{eqn:imh_1}
\pmb{\mathcal{S}}_p = \mathcal{N}(\pmb{\mathcal{S}}_{seed},~\pmb{\sigma})
\end{equation}

where $\pmb{\mathcal{S}}_{seed}$ is the seed state selected from the previously accepted proposed states and $\pmb{\sigma}$ is the vector of standard deviations (one standard deviation for each parameter dimension) specified by the user.

!syntax parameters /Samplers/IndependentGaussianMH

!syntax inputs /Samplers/IndependentGaussianMH

!syntax children /Samplers/IndependentGaussianMH
