# Gaussian

!syntax description /Likelihood/Gaussian

## Overview

The Gaussian likelihood function considering $N$ experimental configurations is given by:

\begin{equation}
    \label{eqn:triso_likelihood}
    \mathcal{L} = \prod_{i=1}^N \mathcal{N}\big(\hat{M}(\pmb{\theta},~\pmb{\Theta}_i) - M(\pmb{\Theta}_i),~\sigma \big)
\end{equation}

where, $\hat{M}(\pmb{\theta},~\pmb{\Theta}_i)$ is the model prediction given model parameters $\pmb{\theta}$ and the $i^{\text{th}}$ experimental configuration $\pmb{\Theta}_i$ and $M(\pmb{\Theta}_i)$ is the $i^{\text{th}}$ experimental data point. $\sigma$ above is the scale of the distribution representing the model inadequacy and experimental noise uncertainties, while $\mathcal{N}$ represents a Gaussian distribution.

## Example Input File Syntax

!listing test/tests/likelihoods/gaussian_derived/main.i block=Likelihood

!syntax parameters /Likelihood/Gaussian

!syntax inputs /Likelihood/Gaussian

!syntax children /Likelihood/Gaussian
