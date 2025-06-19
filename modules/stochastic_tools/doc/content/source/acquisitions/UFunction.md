# UFunction

!syntax description /ParallelAcquisition/UFunction

## Overview

The UFunction acquisition function for parallel active learning (rare events analysis) is given by:

\begin{equation}
    \label{eqn:ei_al}
    a(\pmb{x}) = \frac{|\mu - \psi|}{\sigma}
\end{equation}

where, $\psi$ is a tuning parameter, $\mu$ is the Gaussian process mean prediction, and $\sigma$ is the Gaussian process standard deviation.

!syntax parameters /ParallelAcquisition/UFunction

!syntax inputs /ParallelAcquisition/UFunction

!syntax children /ParallelAcquisition/UFunction
