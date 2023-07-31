# AffineInvariantStretchSampler

!syntax description /Samplers/AffineInvariantStretchSampler

## Overview

The `proposeSamples` function from the [PMCMCBase](PMCMCBase.md) parent class is overriden. This proposal is the stretch move proposed in [!cite](Goodman2010a) which defines an implicit probability distribution from the accepted samples in the previous step and draws samples from it. Note that the `AffineInvariantStretchSampler` will also infer the variance term (i.e., the model inadequacy plus experimental noise uncertainty) if the associated prior is specified. If lower and upper bounds to the parameters are specified, the proposals are passed through a rejection sampling until specified bounds across all the parameter dimensions are satisfied. Specifically, the new $p^{\text{th}}$ parallel proposal is given by:

\begin{equation}
\label{eqn:ss_1}
\mathcal{S}_{y^p} = \mathcal{S}_{y^r_{-1}} + z^p~ (\mathcal{S}_{y^{p}_{-1}} - \mathcal{S}_{y^r_{-1}})
\end{equation}

where $\mathcal{S}_{y^{p}_{-1}}$ is the previous state at index $p$, $\mathcal{S}_{y^r_{-1}}$ is the previous state at a random index $r$ defined between $[1,~P]$ excluding the index $p$, and $z^p$ is a random step size corresponding to the index $p$. $z$ is defined to originate from a probability distribution of the form [!cite](Goodman2010a):

\begin{equation}
    \label{eqn:ss_2}
    f(z) = \begin{cases}
  \frac{1}{\sqrt{z}} & \text{ if } z \in [a,~\frac{1}{a}]~\text{with } a > 1 \\
  0 & \text{ otherwise}
\end{cases}
\end{equation}

[!cite](Goodman2010a) recommend an $a$ value of $2$ for most cases.

!syntax parameters /Samplers/AffineInvariantStretchSampler

!syntax inputs /Samplers/AffineInvariantStretchSampler

!syntax children /Samplers/AffineInvariantStretchSampler
