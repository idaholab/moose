# ExpectedImprovementGlobalFit

!syntax description /ParallelAcquisition/ExpectedImprovementGlobalFit

## Overview

The ExpectedImprovementGlobalFit acquisition function for parallel active learning (global surrogate fitting) is given by [!citep](Lam2008thesis):

\begin{equation}
    \label{eqn:ei_al}
    a(\pmb{x}) = (\mu - f(\pmb{x}^*))^2 + \sigma^2
\end{equation}

where, $f(\pmb{x}^*)$ is the computational model output at $\pmb{x}^*$ which is the closest point to $\pmb{x}$, $\mu$ is the Gaussian process mean prediction, and $\sigma$ is the Gaussian process standard deviation.

!syntax parameters /ParallelAcquisition/ExpectedImprovementGlobalFit

!syntax inputs /ParallelAcquisition/ExpectedImprovementGlobalFit

!syntax children /ParallelAcquisition/ExpectedImprovementGlobalFit

