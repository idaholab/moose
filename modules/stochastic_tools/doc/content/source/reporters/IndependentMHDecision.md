# IndependentMHDecision (Independent Metropolis-Hastings Decision)

!syntax description /Reporters/IndependentMHDecision

## Overview

The `IndependentMHDecision` derives from [PMCMCDecision](PMCMCDecision.md) and overrides both the `computeEvidence` and the `computeTransitionVector` functions. This is due to the fact that Metropolis-Hastings samplers use a single seed to compute the next $P$ proposals and the evidences for all these samples need to be evaluated in reference to the seed sample. As such, any MCMC sampler that follows this single seed principle should derive from `IndependentMHDecision` and should typically only override the `computeTransitionVector` function, depending upon its mathematical construction.

Parallelized Metropolis-Hastings class of samplers is proposed by [!cite](Calderhead2014a). The sampling procedure is similar to that of a serial Metropolis-Hastings sampler with some modifications to account for the parallelization. At each serial step, a seed state $\mathcal{S}_x$ is defined. Using this seed state, $P$ parallel proposals are made using a proposal distribution $\mathcal{G}$. The computational model is evaluated in parallel for these proposals and likelihood function is computed. Then, a transition probability vector $\pmb{t}_{xy}$ is computed whose elements are defined as:

\begin{equation}
    \label{eqn:imh_2}
    \pmb{t}_{xy} = \begin{cases}
  \frac{1}{P} \min{(1,~t_{xy^p})}  & \text{ if } x\neq y  \\
  1-\sum_{x\neq y} \pmb{t}_{xy} & \text{ otherwise}
\end{cases}
\end{equation}

where $t_{xy^p}$ is the transition probability defined as shown in the above equation for the $p^{\text{th}}$ parallel proposal. Each of the $P$ parallel proposals is accepted or rejected with a probability defined by the corresponding index in the vector $\pmb{t}_{xy}$. [!cite](Calderhead2014a) proved the theoretical convergence of this parallelized Metropolis-Hastings construction to the required posterior. In addition, the proposal distribution $\mathcal{G}$ can be defined in a flexible way in that it can be a random-walk proposal, Langevin dynamics [!cite](Cheng2022a) proposal, or Hamiltonian dynamics [!cite](Betancourt2017a) proposal.

!syntax parameters /Reporters/IndependentMHDecision

!syntax inputs /Reporters/IndependentMHDecision

!syntax children /Reporters/IndependentMHDecision
