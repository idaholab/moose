# EshelbyTensor
!syntax description /Materials/EshelbyTensor

## Description

This models computes the Eshelby energy-momentum tensor \cite{eshelby_energy_1999} $\Sigma$, used in fracture integral calculations:

$$
\boldsymbol{\Sigma} = W\boldsymbol{I} - \boldsymbol{H}^T\boldsymbol{P}
$$

where W is the strain energy density in the original configuration, $\boldsymbol{I}$ is the identity matrix, $\boldsymbol{H}$ is the displacement gradient, and $\boldsymbol{P}$ is the first Piola-Kirchoff stress tensor.

It is necessary to define this Material block when computing fracture integrals.

!syntax parameters /Materials/EshelbyTensor

!syntax inputs /Materials/EshelbyTensor

!syntax children /Materials/EshelbyTensor

\bibliography{tensor_mechanics.bib}
