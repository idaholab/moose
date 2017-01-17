#IsotropicPlasticityStressUpdate
!description /Materials/IsotropicPlasticityStressUpdate


## Description
{!docs/content/documentation/modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md!}

In isotropic linear hardening plasticity, with the hardening function $ r = hp$, the effective plastic strain increment has the form:
$$
 d \Delta p = \frac{\sigma^{trial}_{effective} - 3 G \Delta p - r - \sigma_{yield}}{3G + h}
$$
where G is the isotropic shear modulus, and $\sigma^{trial}_{effective}$ is the scalar von Mises trial stress.

This class calculates an effective trial stress, an effective scalar plastic strain increment, and the derivative of the scalar effective plastic strain increment; these values are passed to the RecomputeRadialReturn to compute the radial return stress increment.  This isotropic plasticity class also computes the plastic strain as a stateful material property.

This class is based on the implicit integration algorithm in \cite{dunne2005introduction} pg. 146 - 149.

!parameters /Materials/IsotropicPlasticityStressUpdate

!inputfiles /Materials/IsotropicPlasticityStressUpdate

!childobjects /Materials/IsotropicPlasticityStressUpdate

## References
\bibliographystyle{unsrt}
\bibliography{docs/bib/tensor_mechanics.bib}
