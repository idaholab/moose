# Hill Elasto-Plasticity Stress Update

!syntax description /Materials/ADHillElastoPlasticityStressUpdate

## Description

This class computes Hill plasticity via a generalized radial return mapping algorithm [!cite](versino2018generalized). This object implements an algorithm indicated for anisotropic elastoplasticity, i.e. a combination of elastic anisotropy plus a yield function where each stress component has its own yield value.

The Hill yield function can be defined as
\begin{equation}
f = \frac{1}{2} \boldsymbol{\sigma}^{T} \boldsymbol{A} \boldsymbol{\sigma}^{T} - \sigma_{y}(\alpha)
\end{equation}
where $\boldsymbol{\sigma}$ is the Cauchy stress tensor in Voigt form, $\boldsymbol{A}$ is the anisotropy (Hill) tensor, and $\alpha$ is an internal parameter that can be used, for example, to prescribe strain hardening through a plasticity modulus. Note that the Hill tensor is defined as a six by six matrix using the following unitless constants: $F$, $G$, $H$, $L$, $M$ and $N$.

!alert warning
The combination of elastic isotropy and plastic anisotropy should be solved by the more efficient [ADHillPlasticityStressUpdate](/ADHillPlasticityStressUpdate.md) class.

The effective plastic strain increment is obtained within the framework of a generalized (Hill plasticity) radial return mapping, see
[ADGeneralizedRadialReturnStressUpdate](/ADGeneralizedRadialReturnStressUpdate.md).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_elastoplasticity/ad_aniso_plasticity_x_one.i block=Materials/trial_plasticity

!syntax parameters /Materials/ADHillElastoPlasticityStressUpdate

!syntax inputs /Materials/ADHillElastoPlasticityStressUpdate

!syntax children /Materials/ADHillElastoPlasticityStressUpdate

!bibtex bibliography
