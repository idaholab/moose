# Transversely Isotropic Plasticity Stress Update

!syntax description /Materials/ADTransverselyIsotropicPlasticityStressUpdate

## Description

This class computes Hill plasticity via a generalized radial return mapping algorithm [!cite](versino2018generalized). It
requires that the elastic behavior of the material is isotropic, whereas any departure from the yield function is anisotropic.
The Hill yield function can be defined as
\begin{equation}
f = \frac{1}{2} \boldsymbol{s}^{T} \boldsymbol{A} \boldsymbol{s}^{T} - s_{y}(\alpha)
\end{equation}
where $\boldsymbol{s}$ is the stress tensor, $\boldsymbol{A}$ is the anisotropy (Hill) tensor, and $\alpha$ is an internal parameter that can be used, for example, to prescribe strain hardening through a plasticity modulus. Note that the Hill tensor is defined as a six by six matrix using the following unitless constants: $F$, $G$, $H$, $L$, $M$ and $N$.

!alert warning
The combination of elastic anisotropy and plastic anisotropy requires further tensor projections and is scheduled to be implemented soon.

The effective plastic strain increment is obtained within the framework of a generalized (Hill plasticity) radial return mapping, see
[ADGeneralizedRadialReturnStressUpdate](/ADGeneralizedRadialReturnStressUpdate.md).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ad_anisotropic_plasticity/anis_plasticity_test_experiment.i block=Materials/trial_plasticity

!syntax parameters /Materials/ADTransverselyIsotropicPlasticityStressUpdate

!syntax inputs /Materials/ADTransverselyIsotropicPlasticityStressUpdate

!syntax children /Materials/ADTransverselyIsotropicPlasticityStressUpdate

!bibtex bibliography
