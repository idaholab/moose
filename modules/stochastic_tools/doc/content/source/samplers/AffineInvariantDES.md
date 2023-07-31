# AffineInvariantDES (AffineInvariantDifferential)

!syntax description /Samplers/AffineInvariantDES

## Overview

The `proposeSamples` function from the [PMCMCBase](PMCMCBase.md) parent class is overriden. This proposal is the differential evolution move proposed in [!cite](Braak2006a) which defines an implicit probability distribution from the accepted samples in the previous step and draws samples from it. Note that the `AffineInvariantDES` will also infer the variance term (i.e., the model inadequacy plus experimental noise uncertainty) if the associated prior is specified. If lower and upper bounds to the parameters are specified, the proposals are passed through a rejection sampling until specified bounds across all the parameter dimensions are satisfied. Specifically, the new $p^{\text{th}}$ parallel proposal is given by:

\begin{equation}
\label{eqn:des_1}
\mathcal{S}_{y^p} = \mathcal{S}_{y^{p}_{-1}} + \gamma ~(\mathcal{S}_{y^{r1}_{-1}} - \mathcal{S}_{y^{r2}_{-1}}) + \nu
\end{equation}

where $r1$ is a random index defined between $[1,~P]$ excluding the index $p$ and $r2$ is a random index defined between $[1,~P]$ excluding the indices $p$ and $r1$. Also, $\gamma = 2.38 / \sqrt{2M}$ and $\nu \sim \mathcal{N}(0,~1e-6)$.

!syntax parameters /Samplers/AffineInvariantDES

!syntax inputs /Samplers/AffineInvariantDES

!syntax children /Samplers/AffineInvariantDES
