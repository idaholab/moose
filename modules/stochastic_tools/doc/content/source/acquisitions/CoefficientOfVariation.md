# CoefficientOfVariation

!syntax description /ParallelAcquisition/CoefficientOfVariation

## Overview

The CoefficientOfVariation acquisition function for parallel active learning is given by:

\begin{equation}
    \label{eqn:cov_al}
    a(\pmb{x}) = \frac{\sigma}{\mu}
\end{equation}

where, $\mu$ is the Gaussian process mean prediction and $\sigma$ is the Gaussian process standard deviation.

!syntax parameters /ParallelAcquisition/CoefficientOfVariation

!syntax inputs /ParallelAcquisition/CoefficientOfVariation

!syntax children /ParallelAcquisition/CoefficientOfVariation
