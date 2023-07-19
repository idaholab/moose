# TruncatedGaussian

!syntax description /Likelihood/TruncatedGaussian

## Overview

The truncated Gaussian likelihood function considering $N$ experimental configurations is given by:

\begin{equation}
    \label{eqn:triso_likelihood}
    \mathcal{L} = \prod_{i=1}^N \mathcal{TN}\big(\hat{M}(\pmb{\theta},~\pmb{\Theta}_i) - M(\pmb{\Theta}_i),~\sigma,~lb,~ub \big)
\end{equation}

where, $\hat{M}(\pmb{\theta},~\pmb{\Theta}_i)$ is the model prediction given model parameters $\pmb{\theta}$ and the $i^{\text{th}}$ experimental configuration $\pmb{\Theta}_i$ and $M(\pmb{\Theta}_i)$ is the $i^{\text{th}}$ experimental data point. $\sigma$ above the scale of the distribution representing the model inadequacy and experimental noise uncertainties.  $\mathcal{TN}$ represents a truncated Gaussian distribution with $lb$ and $ub$ lower and upper bounds, respectively.

!syntax parameters /Likelihood/TruncatedGaussian

!syntax inputs /Likelihood/TruncatedGaussian

!syntax children /Likelihood/TruncatedGaussian
