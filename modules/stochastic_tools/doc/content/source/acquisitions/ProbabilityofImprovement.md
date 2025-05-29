# ProbabilityofImprovement

!syntax description /ParallelAcquisition/ProbabilityofImprovement

## Overview

The ProbabilityofImprovement acquisition function for parallel active learning (Bayesian optimization) is given by:

\begin{equation}
    \label{eqn:ei_al}
    a(\pmb{x}) = \Phi\Big(\frac{\mu-f(\pmb{x}^*)}{\sigma}\Big)
\end{equation}

where, $f(\pmb{x}^*)$ is the computational model output at the best point thus far $\pmb{x}^*$, $\Phi$ is a standard Normal CDF, $\mu$ is the Gaussian process mean prediction, and $\sigma$ is the Gaussian process standard deviation.

!syntax parameters /ParallelAcquisition/ProbabilityofImprovement

!syntax inputs /ParallelAcquisition/ProbabilityofImprovement

!syntax children /ParallelAcquisition/ProbabilityofImprovement
