# CoefficientofVariation

!syntax description /ParallelAcquisition/CoefficientofVariation

## Overview

The CoefficientofVariation acquisition function for parallel active learning is given by:

\begin{equation}
    \label{eqn:cov_al}
    a(\pmb{x}) = \frac{\sigma}{\mu}
\end{equation}

where, $\mu$ is the Gaussian process mean prediction and $\sigma$ is the Gaussian process standard deviation.

!syntax parameters /ParallelAcquisition/CoefficientofVariation

!syntax inputs /ParallelAcquisition/CoefficientofVariation

!syntax children /ParallelAcquisition/CoefficientofVariation
