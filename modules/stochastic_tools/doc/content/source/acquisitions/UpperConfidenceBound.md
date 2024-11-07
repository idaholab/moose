# UpperConfidenceBound

!syntax description /ParallelAcquisition/UpperConfidenceBound

## Overview

The UpperConfidenceBound acquisition function for parallel active learning (Bayesian optimization) is given by:

\begin{equation}
    \label{eqn:ei_al}
    a(\pmb{x}) = \mu + \psi~\sigma
\end{equation}

where, $\psi$ is a tuning parameter to boost exploration or exploitation, $\mu$ is the Gaussian process mean prediction, and $\sigma$ is the Gaussian process standard deviation.

!syntax parameters /ParallelAcquisition/UpperConfidenceBound

!syntax inputs /ParallelAcquisition/UpperConfidenceBound

!syntax children /ParallelAcquisition/UpperConfidenceBound
