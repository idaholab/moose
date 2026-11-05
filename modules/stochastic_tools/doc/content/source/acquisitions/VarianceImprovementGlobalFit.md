# VarianceImprovementGlobalFit

!syntax description /ParallelAcquisition/VarianceImprovementGlobalFit

## Overview

The VarianceImprovementGlobalFit (VIGF) acquisition function for parallel active learning (global surrogate fitting) is given by [!citep](Mohammadi2024):

\begin{equation}
    \label{eqn:vigf_al}
    a(\pmb{x}) = 4\sigma^2(\mu - f(\pmb{x}^*))^2 + 2\sigma^4
\end{equation}

where, $f(\pmb{x}^*)$ is the computational model output at $\pmb{x}^*$ which is the closest point to $\pmb{x}$, $\mu$ is the Gaussian process mean prediction, and $\sigma$ is the Gaussian process standard deviation.


!syntax parameters /ParallelAcquisition/VarianceImprovementGlobalFit

!syntax inputs /ParallelAcquisition/VarianceImprovementGlobalFit

!syntax children /ParallelAcquisition/VarianceImprovementGlobalFit
