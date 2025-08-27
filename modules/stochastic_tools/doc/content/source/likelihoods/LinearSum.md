# LinearSum

!syntax description /Likelihood/LinearSum

## Overview

The LinearSum likelihood function considering $N$ experimental configurations is given by:

\begin{equation}
    \label{eqn:triso_likelihood}
    \mathcal{L} = \sum_{i=1}^N \Big(\frac{\hat{M}(\pmb{\theta},~\pmb{\Theta}_i) - M(\pmb{\Theta}_i)}{\sigma}\Big)^2
\end{equation}

where, $\hat{M}(\pmb{\theta},~\pmb{\Theta}_i)$ is the model prediction given model parameters $\pmb{\theta}$ and the $i^{\text{th}}$ experimental configuration $\pmb{\Theta}_i$ and $M(\pmb{\Theta}_i)$ is the $i^{\text{th}}$ experimental data point. $\sigma$ above is the scale of the distribution representing the model inadequacy and experimental noise uncertainties.

## Example Input File Syntax

!listing test/tests/likelihoods/gaussian_derived/main.i block=Likelihood

!syntax parameters /Likelihood/LinearSum

!syntax inputs /Likelihood/LinearSum

!syntax children /Likelihood/LinearSum
