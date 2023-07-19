# AffineInvariantDifferentialDecision

!syntax description /Reporters/AffineInvariantDifferentialDecision

## Overview

The `AffineInvariantDifferentialDecision` class derives from [PMCMCDecision](PMCMCDecision.md) and only overrides the `computeTransitionVector` function. This is to implement the acceptance probability computation step defined by the equation [!cite](Braak2006a):

\begin{equation}
    \label{eqn:ens_5}
    t_{\mathcal{S}_{y^{p}_{-1}} \mathcal{S}_{y^p}} = \min{\bigg\{1,~\frac{\mathcal{L}(\mathcal{S}_{y^p}) \mathcal{P}(\mathcal{S}_{y^p})}{\mathcal{L}(\mathcal{S}_{y^{p}_{-1}}) \mathcal{P}(\mathcal{S}_{y^{p}_{-1}})}\bigg\}}
\end{equation}

where $\mathcal{S}_{y^{p}_{-1}}$ the accepted state at index $p$ from the previous step, $\mathcal{S}_{y^{p}}$ the proposed state at index $p$ from the current step, $\mathcal{L}$ is the likelihood function, and $\mathcal{P}$ is the prior distribution.

!syntax parameters /Reporters/AffineInvariantDifferentialDecision

!syntax inputs /Reporters/AffineInvariantDifferentialDecision

!syntax children /Reporters/AffineInvariantDifferentialDecision
