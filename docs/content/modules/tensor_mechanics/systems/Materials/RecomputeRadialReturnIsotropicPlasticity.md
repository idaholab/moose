#Recompute Radial Return Isotropic Plasticity
!description /Materials/RecomputeRadialReturnIsotropicPlasticity

!devel /Materials/RecomputeRadialReturnIsotropicPlasticity float=right width=auto margin=20px padding=20px background-color=#F8F8F8

## Description
{!content/modules/tensor_mechanics/common_documentation/supplementalRecomputeRadialReturn.md!}

In isotropic linear hardening plasticity, with the hardening function $ r = hp$, the effective plastic strain increment has the form:
$$
 d \Delta p = \frac{\sigma^{trial}_{effective} - 3 G \Delta p - r - \sigma_{yield}}{3G + h}
$$
where G is the isotropic shear modulus, and $\sigma^{trial}_{effective}$ is the scalar von Mises trial stress.

This class calculates an effective trial stress, an effective scalar plastic strain increment, and the derivative of the scalar effective plastic strain increment; these values are passed to the RecomputeRadialReturn to compute the radial return stress increment.  This isotropic plasticity class also computes the plastic strain as a stateful material property.

This class is based on the implicit integration algorithm in F. Dunne and N. Petrinic's Introduction to Computational Plasticity (2004) Oxford University Press, pg. 146 - 149.

!parameters /Materials/RecomputeRadialReturnIsotropicPlasticity

!inputfiles /Materials/RecomputeRadialReturnIsotropicPlasticity

!childobjects /Materials/RecomputeRadialReturnIsotropicPlasticity
