#RadialReturnStressUpdate
!description /Materials/RadialReturnStressUpdate


###Algorithm References
The radial return mapping method, introduced by Simo and Taylor (1985), uses a von Mises yield surface to determine the increment of plastic strain necessary to return the stress state to the yield surface after a trial stress increment takes the computed stress state across the yield surface.  Because the von Mises yield surface in the deviatoric stress space has the shape of a circle, the _plastic correction stress_ is always directed towards the center of the yield surface circle.

In addition to the Sumo and Hughes textbook \cite{simo2006computational}, _Introduction to Computational Plasticity_ by Dunnu and Petrinic (2004) is an excellent reference for users working with the Radial Return Stress Update materials; several of the isotropic plasticity and creep effective plastic strain increment algorithms are taken from \cite{dunne2005introduction}.

###The Radial Return Stress Update Description
The stress update materials are not called by MOOSE directly but instead only by other materials using the `computeProperties` method.  For the `RadialReturnStressUpdate` materials, this calling material is [ComputeReturnMappingStress](ComputeReturnMappingStress.md).  Separating the call to the stress update materials from MOOSE allows us to iteratively call the stress update materials as is required to achieve convergence.

##Radial Return Algorithm Overview
!media docs/media/tensor_mechanics-RadialReturnStressSpace.png width=350 float=right caption=Figure 1: A trial stress is shown outside of the deviatoric yield surface and the radial return stress which is normal to the yield surface.

!include docs/content/documentation/modules/tensor_mechanics/common/supplementalRadialReturnStressUpdate.md

In the case of isotropic linear hardening plasticity, with the hardening function $ r = hp$, the effective plastic strain increment has the form:
$$
 d \Delta p = \frac{\sigma^{trial}_{effective} - 3 G \Delta p - r - \sigma_{yield}}{3G + h}
$$
where G is the isotropic shear modulus, and $\sigma^{trial}_{effective}$ is the scalar von Mises trial stress.

Once convergence has been reached on the scalar inelastic strain increment, the full inelastic strain tensor is calculated.
$$
\Delta \epsilon^{inelastic}_{ij} = \frac{3}{2} \Delta p^{(t+1)} \frac{dev(\sigma^{trial}_{ij})}{\sigma^{trial}_{effective}}
$$

The elastic strain is calculated by subtracting the return mapping inelastic strain increment tensor from the mechanical strain tensor.  Mechanical strain is considered as the sum of the elastic and inelastic (plastic, creep, ect) strains.
$$
\epsilon_{total} = \epsilon_{mechanical} + \epsilon_{eigenstrain}
= \left( \epsilon_{elastic} + \epsilon_{inelastic} \right) + \epsilon_{eigenstrain}
= \epsilon_{elastic} + \left( \epsilon_{plastic} + \epsilon_{creep} + \epsilon_{damage}  \right) + \epsilon_{eigenstrain}
$$

The final inelastic strain is returned from the radial return stress update material, and `ComputeReturnMappingStress` computes the stress, with a return mapping stress increment following elasticity theory for finite strains. The final stress is calculated from the elastic strain increment.
$$
\sigma^{new}_{ij} = C_{ijkl} \left( \Delta \epsilon^{elastic}_{kl} + \epsilon^{old-elastic}_{kl} \right)
$$

When more than one radial recompute material is included in the simulation, as in Combined Power Law Creep and Linear Strain Hardening, `ComputeReturnMappingStress` will iterate over the change in the calculated stress until the return stress has reached a stable value.

Users can print out any of these strains and stresses using the `RankTwoAux` as described on the
[Introduction/Visualizing Tensors](auto::/introduction/VisualizingTensors) page.

##Writing a New Stress Update Material
New radial return models must inherit from `RadialReturnStressUpdate` and must overwrite the six virtual methods.

  - **initQpStatefulProperties**: Set the initial values for all new material properties that are not initialized by an input parameter; generally the material properties initialized in this method are all set to zero.
  - **computeStressInitialize**: Calculate the initial trial stress state, the yield surface value, and any hardening or softening parameters at the start of the simulation time increment.
  - **computeResidual**: In each iteration over the inelastic strain increment, calculate the value of the effective scalar trial stress subtracted by the yield surface function.
  - **computeDerivative**: In each iteration over the inelastic strain increment, calculate the derivative of the yield surface function with respect to the inelastic strain increment.

  - **iterationFinalize**: Store the value of the inelastic strain increment at the end of each iteration.
  - **computeStressFinalize**: Update the stress after convergence on the inelastic strain increment has been reached.

Additionally, new radial return methods must also overwrite a single method from the MOOSE `Material` class.

  - **resetQpProperties**: Set the material property used in the iteration, usually $ \Delta p $, to zero at the start the iteration.  This method is necessary to avoid incorrect material property values.

More details on how to write the equivalent yield surface equation for a creep model are given in Dunne and Petrinic.


!childobjects /Materials/RadialReturnStressUpdate

## References
\bibliographystyle{unsrt}
\bibliography{docs/bib/tensor_mechanics.bib}
