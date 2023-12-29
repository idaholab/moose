# Isotropic Plasticity Stress Update

!syntax description /Materials/IsotropicPlasticityStressUpdate

## Description

!include modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md

In isotropic linear hardening plasticity, with the hardening function $r = hp$, the effective
plastic strain increment has the form:
\begin{equation}
 d \Delta p = \frac{\sigma^{trial}_{effective} - 3 G \Delta p - r - \sigma_{yield}}{3G + h}
\end{equation}
where $G$ is the isotropic shear modulus, and $\sigma^{trial}_{effective}$ is the scalar von Mises
trial stress.

This class calculates an effective trial stress, an effective scalar plastic strain increment, and
the derivative of the scalar effective plastic strain increment; these values are passed to the
[RadialReturnStressUpdate](/RadialReturnStressUpdate.md) to compute the radial return stress
increment.  This isotropic plasticity class also computes the plastic strain as a stateful material
property.

This class is based on the implicit integration algorithm in [!cite](dunne2005introduction)
pg. 146--149.  

The `ADIsotropicPlasticityStressUpdate` version of this class uses forward mode automatic
differentiation to provide all necessary material property derivatives to
assemble a perfect Jacobian (this replaces the approximated tangent operator).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/recompute_radial_return/isotropic_plasticity_finite_strain.i block=Materials/isotropic_plasticity

`IsotropicPlasticityStressUpdate` must be run in conjunction with the inelastic strain return mapping
stress calculator as shown below:

!listing modules/tensor_mechanics/test/tests/recompute_radial_return/isotropic_plasticity_finite_strain.i block=Materials/radial_return_stress

!syntax parameters /Materials/IsotropicPlasticityStressUpdate

!syntax inputs /Materials/IsotropicPlasticityStressUpdate

!syntax children /Materials/IsotropicPlasticityStressUpdate

!bibtex bibliography
