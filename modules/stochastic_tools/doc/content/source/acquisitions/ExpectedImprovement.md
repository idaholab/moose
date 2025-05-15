# ExpectedImprovement

!syntax description /ParallelAcquisition/ExpectedImprovement

## Overview

The ExpectedImprovement acquisition function for parallel active learning (Bayesian optimization) is given by:

\begin{equation}
    \label{eqn:ei_al}
    a(\pmb{x}) = (\mu-f(\pmb{x}^*)-\psi)~\Phi\Big(\frac{\mu-f(\pmb{x}^*)-\psi}{\sigma}\Big)+\sigma~\phi\Big(\frac{\mu-f(\pmb{x}^*)-\psi}{\sigma}\Big)
\end{equation}

where, $\psi$ is a tuning parameter to boost exploration or exploitation, $f(\pmb{x}^*)$ is the computational model output at the best point thus far $\pmb{x}^*$, $\Phi$ is a standard Normal CDF, $\phi$ is a standard Normal PDF, $\mu$ is the Gaussian process mean prediction, and $\sigma$ is the Gaussian process standard deviation.

!syntax parameters /ParallelAcquisition/ExpectedImprovement

!syntax inputs /ParallelAcquisition/ExpectedImprovement

!syntax children /ParallelAcquisition/ExpectedImprovement
