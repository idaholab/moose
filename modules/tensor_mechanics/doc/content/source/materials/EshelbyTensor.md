# Eshelby Tensor

!syntax description /Materials/EshelbyTensor

## Description

This model computes the Eshelby energy-momentum tensor $\Sigma$ [!citep](eshelby_energy_1999),
used in fracture integral calculations:
\begin{equation}
\boldsymbol{\Sigma} = W\boldsymbol{I} - \boldsymbol{H}^T\boldsymbol{P}
\end{equation}
where W is the strain energy density in the original configuration, $\boldsymbol{I}$
is the identity matrix, $\boldsymbol{H}$ is the displacement gradient, and
$\boldsymbol{P}$ is the first Piola-Kirchoff stress tensor.

It is necessary to include this material within the input file when computing
fracture integrals.

## Example Input File Syntax

!syntax parameters /Materials/EshelbyTensor

!syntax inputs /Materials/EshelbyTensor

!syntax children /Materials/EshelbyTensor
