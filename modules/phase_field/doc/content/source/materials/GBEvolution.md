# GBEvolution

!syntax description /Materials/GBEvolution

## Overview

This material calculates the order parameter mobility $L$, the free energy prefactor $\mu$, and the gradient multiplier $\kappa$ for the grain growth model from [!cite](moelans_quantitative_2008), assuming isotropic grain boundary (GB) properties and a symmetric interfacial profile ($\gamma=1.5$). The parameters are calculated based on the GB energy $\sigma$, the GB mobility $M_{GB}$, and the phase field interfacial width $w_{GB}$ according to

\begin{equation}
L = \frac{4}{3} \frac{M_{GB}}{w_{GB}}
\end{equation}
\begin{equation}
\mu = 6 \frac{\sigma}{w_{GB}}
\end{equation}
\begin{equation}
\kappa = \frac{3}{4} \sigma w_{GB}.
\end{equation}

The GB mobility can be defined in terms of the temperature $T$ using an Arrhenius expression

\begin{equation}
  M_{GB} = M_0 \exp\left( -\frac{Q}{k_b T} \right),
\end{equation}
where $M_0$ is the mobility prefactor, $Q$ is the activation energy, and $k_b$ is the Boltzman constant.

## Example Input File Syntax

!listing modules/phase_field/test/tests/grain_growth/evolution.i block=Moly_GB

!syntax parameters /Materials/GBEvolution

!syntax inputs /Materials/GBEvolution

!syntax children /Materials/GBEvolution
