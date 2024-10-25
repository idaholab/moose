# Combined Nonlinear Hardening Plasticity

!syntax description /Materials/CombinedNonlinearHardeningPlasticity

## Description


!include modules/solid_mechanics/common/supplementalRadialReturnStressUpdate.md

In this model, the linear isotropic hardening function is given by $r = Q(1-exp(-bp))$, and kinematic
hardening is governed by a backstress evolution equation of the form: $dX = Cdp - DXp$, where X is the backstress, and the two coefficients, C and D, represent the evolution of kinematic hardening.

The non-linear equations for Isotropic and Kinematic Hardening are based on
[!cite](besson2009non)
pg. 82--84. The reference uses the nonlinear kinematic hardening rule proposed by Armstrong and Frederick [!cite](armstrong1966mathematical), while the isotropic hardening rule follows the Voce isotropic hardening model [!cite](voce1948relationship).

The effective plastic strain
increment for combined hardening has the form:
\begin{equation}
 d \Delta p = \frac{\sigma^{trial}_{effective} - 3 G \Delta p - r - \sigma_{yield}}{3G + h}
\end{equation}
where $G$ is the isotropic shear modulus, and $\sigma^{trial}_{effective}$ is the scalar part
of (von Mises trial stress - backstress).

The non-linear equations for Isotropic and Kinematic Hardening are based on
[!cite](besson2009non)
pg. 82--84.

This class calculates an effective trial stress, an effective scalar plastic strain increment, and
the derivative of the scalar effective plastic strain increment; these values are passed to the
RadialReturnBackstressStressUpdateBase to compute the radial return stress
increment.  This class also computes the plastic strain as a stateful material
property.

This class is based on the implicit integration algorithm in [!cite](dunne2005introduction)
pg. 146--149.

The `ADCombinedNonlinearHardeningPlasticity` version of this class uses forward mode automatic
differentiation to provide all necessary material property derivatives to
assemble a perfect Jacobian (this replaces the approximated tangent operator).

This model can be reduced to a purely linear isotropic model, if the kinematic hardening modulus is set to zero, as the backstress in the effective trial stress will remain zero and will not be updated. Conversely, if only the kinematic hardening modulus is set to a nonzero value, the hardening model will behave purely as linear kinematic.

To model the nonlinear hardening behavior of a material, the constants Q (saturation hardening) and b (rate of hardening) should be set to nonzero values for nonlinear isotropic hardening. Additionally, to capture nonlinear kinematic hardening behavior, the kinematic saturation gamma should be nonzero. For a combined nonlinear hardening model, all constants (Q, b, and gamma) should be set to nonzero values.

The following cyclic stress-strain plots below verify some of the responses

This combined hardening model is verified by plotting cyclic stress-strain curves using the same values of constants from [!cite](besson2009non) pg. 87--90. The model's results are compared to those from that reference, confirming that the implementation is functioning as expected. Some of the examples of cyclic hardening plots are shown below:

!media media/solid_mechanics/rate_independent_cyclic_hardening/non_linear_isotropic_symmetric_strain_controlled.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:non_linear_isotropic_symmetric_strain_controlled
       caption=Isotropic Hardening under prescribed symmetric strain

!media media/solid_mechanics/rate_independent_cyclic_hardening/non_linear_combined_symmetric_strain_controlled.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:non_linear_combined_symmetric_strain_controlled
       caption=Isotropic and Kinematic Nonlinear Hardening under prescribed symmetric strain

!media media/solid_mechanics/rate_independent_cyclic_hardening/non_linear_kinematic_nonsymmetric_strain_controlled.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:non_linear_kinematic_nonsymmetric_strain_controlled
       caption=Nonlinear Kinematic Hardening under nonsymmetric imposed strain

!media media/solid_mechanics/rate_independent_cyclic_hardening/1d_ratcheting_non_linear_kinematic_load_controlled.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:1d_ratcheting_non_linear_kinematic_load_controlled
       caption=1D Ratcheting under nonsymmetrical imposed stress path due to nonlinear kinematic hardening

## Example Input File Syntax

!listing modules/solid_mechanics/test/tests/rate_independent_cyclic_hardening/nonlin_isokinharden_symmetric_strain_controlled.i block=Materials/combined_plasticity

`CombinedNonlinearHardeningPlasticity` must be used in conjunction with this inelastic strain return mapping stress calculator as shown below:

!listing modules/solid_mechanics/test/tests/rate_independent_cyclic_hardening/nonlin_isokinharden_symmetric_strain_controlled.i block=Materials/radial_return_stress

!syntax parameters /Materials/CombinedNonlinearHardeningPlasticity

!syntax inputs /Materials/CombinedNonlinearHardeningPlasticity

!syntax children /Materials/CombinedNonlinearHardeningPlasticity

!bibtex bibliography
