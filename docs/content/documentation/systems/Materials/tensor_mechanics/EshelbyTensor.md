# Eshelby Tensor
!syntax description /Materials/EshelbyTensor

## Description
This model computes the Eshelby energy-momentum tensor $\Sigma$ \cite{eshelby_energy_1999}, used in fracture integral calculations:
\begin{equation}
\boldsymbol{\Sigma} = W\mathbf{I} - \mathbf{H}^T\mathbf{P}
\end{equation}
where W is the strain energy density in the original configuration, $\mathbf{I}$ is the identity matrix, $\mathbf{H}$ is the displacement gradient, and $\mathbf{P}$ is the first Piola-Kirchoff stress tensor.

It is necessary to include this material within the input file when computing fracture integrals.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/j_integral/j_integral_2d.i block=Materials/eshelby

!syntax parameters /Materials/EshelbyTensor

!syntax inputs /Materials/EshelbyTensor

!syntax children /Materials/EshelbyTensor

## References
\bibliography{tensor_mechanics.bib}
