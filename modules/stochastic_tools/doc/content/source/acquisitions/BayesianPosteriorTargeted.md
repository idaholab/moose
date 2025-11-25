# BayesianPosteriorTargeted

!syntax description /ParallelAcquisition/BayesianPosteriorTargeted

## Overview

The BayesianPosteriorTargeted acquisition function is proposed by [!citep](Gammal2023al) for Bayesian inverse UQ applications. The functional form is given by:

\begin{equation}
    \label{eqn:posterior_al}
    a(\pmb{x}) = \exp{(2\psi \mu)}~(\exp{(\sigma)}-1)
\end{equation}

where, $\psi$ is a factor to boost the exploratory behavior and set to $N^{-0.85}$ ($N$ is the number of model parameters), $\mu$ is the Gaussian process mean prediction, and $\sigma$ is the Gaussian process standard deviation.

!syntax parameters /ParallelAcquisition/BayesianPosteriorTargeted

!syntax inputs /ParallelAcquisition/BayesianPosteriorTargeted

!syntax children /ParallelAcquisition/BayesianPosteriorTargeted
