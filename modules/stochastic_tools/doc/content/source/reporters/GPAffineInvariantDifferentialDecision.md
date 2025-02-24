# GPAffineInvariantDifferentialDecision

!syntax description /Reporters/GPAffineInvariantDifferentialDecision

## Overview

The `GPAffineInvariantDifferentialDecision` class derives from [PMCMCDecision](PMCMCDecision.md) and is similar to the [AffineInvariantDifferentialDecision](AffineInvariantDifferentialDecision.md). The difference is it supports the usage of a pre-trained Gaussian process instead of calling the computational model for Bayesian uncertainty quantification. See [GenericActiveLearner](GenericActiveLearner.md) for more details on active learning. Specifically, it uses the Gaussian process predictions to compute the acceptance probability defined by the equation [!cite](Braak2006a):

\begin{equation}
    \label{eqn:ens_5}
    t_{\mathcal{S}_{y^{p}_{-1}} \mathcal{S}_{y^p}} = \min{\bigg\{1,~\frac{\mathcal{L}(\mathcal{S}_{y^p}) \mathcal{P}(\mathcal{S}_{y^p})}{\mathcal{L}(\mathcal{S}_{y^{p}_{-1}}) \mathcal{P}(\mathcal{S}_{y^{p}_{-1}})}\bigg\}}
\end{equation}

where $\mathcal{S}_{y^{p}_{-1}}$ the accepted state at index $p$ from the previous step, $\mathcal{S}_{y^{p}}$ the proposed state at index $p$ from the current step, $\mathcal{L}$ is the likelihood function, and $\mathcal{P}$ is the prior distribution.

!syntax parameters /Reporters/GPAffineInvariantDifferentialDecision

!syntax inputs /Reporters/GPAffineInvariantDifferentialDecision

!syntax children /Reporters/GPAffineInvariantDifferentialDecision
