# Hill Plasticity Stress Update

!syntax description /Materials/HillPlasticityStressUpdate

## Description

This class computes Hill plasticity via a generalized radial return mapping algorithm [!cite](versino2018generalized). It
requires that the elastic behavior of the material is isotropic, whereas any departure from the yield function is anisotropic.
The Hill yield function can be defined as
\begin{equation}
f = \frac{1}{2} \boldsymbol{s}^{T} \boldsymbol{A} \boldsymbol{s}^{T} - s_{y}(\alpha)
\end{equation}
where $\boldsymbol{s}$ is the deviatoric stress tensor in vector form, $\boldsymbol{A}$ is the anisotropy (Hill) tensor, and $\alpha$ is an internal parameter that can be used, for example, to prescribe strain hardening through a plasticity modulus. Note that the Hill tensor is defined as a six by six matrix using the following unitless constants: $F$, $G$, $H$, $L$, $M$ and $N$.

!alert warning
The combination of elastic anisotropy and plastic anisotropy requires further tensor projections. Use `HillElastoPlasticityStressUpdate` for that application.

The effective plastic strain increment is obtained within the framework of a generalized (Hill plasticity) radial return mapping, see
[GeneralizedRadialReturnStressUpdate](/GeneralizedRadialReturnStressUpdate.md).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/anisotropic_plasticity/ad_aniso_plasticity_y.i block=Materials/trial_plasticity

!syntax parameters /Materials/HillPlasticityStressUpdate

!syntax inputs /Materials/HillPlasticityStressUpdate

!syntax children /Materials/HillPlasticityStressUpdate

!bibtex bibliography
