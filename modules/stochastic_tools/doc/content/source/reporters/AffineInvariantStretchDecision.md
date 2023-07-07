# AffineInvariantStretchDecision

!syntax description /Reporters/AffineInvariantStretchDecision

The `AffineInvariantStretchDecision` class derives off of [ParallelMarkovChainMonteCarloDecision](ParallelMarkovChainMonteCarloDecision.md) and only overrides the `computeTransitionVector` function. This is to implement the acceptance probability computation step defined by the equation [!cite](Goodman2010a):

\begin{equation}
    \label{eqn:ens_3}
    t_{\mathcal{S}_{y^{p}_{-1}} \mathcal{S}_{y^p}} = \min{\bigg\{1,~(z^p)^{(M+1)-1}~\frac{\mathcal{L}(\mathcal{S}_{y^p}) \mathcal{P}(\mathcal{S}_{y^p})}{\mathcal{L}(\mathcal{S}_{y^{p}_{-1}}) \mathcal{P}(\mathcal{S}_{y^{p}_{-1}})}\bigg\}}
\end{equation}

where $\mathcal{S}_{y^{p}_{-1}}$ the accepted state at index $p$ from the previous step.

!syntax parameters /Reporters/AffineInvariantStretchDecision

!syntax inputs /Reporters/AffineInvariantStretchDecision

!syntax children /Reporters/AffineInvariantStretchDecision
