# Compute Linear Viscoelastic Stress

!syntax description /Materials/ComputeLinearViscoelasticStress

## Description

This computes the stress of a linear viscoelastic material using a total small strain approximation. The stress is calculated from the mechanical strain and the creep strain as:
\begin{equation}
\sigma_{ij} = C_{ijkl} \left( \epsilon^{mech}_{kl} - \epsilon^{creep}_{kl} \right)
\end{equation}

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/visco/visco_small_strain.i block=Materials/stress

The mechanical strain must be computed with a [ComputeSmallStrain](/ComputeSmallStrain.md) material.

!listing modules/tensor_mechanics/test/tests/visco/visco_small_strain.i block=Materials/strain

The creep strain itself is computed by a linear viscoelastic material such as a [GeneralizedKelvinVoigtModel](/GeneralizedKelvinVoigtModel.md) or a [GeneralizedMaxwellModel](/GeneralizedMaxwellModel.md) material. The elasticity tensor $C_{ijkl}$ is also provided by the same linear viscoelastic material.

!listing modules/tensor_mechanics/test/tests/visco/visco_small_strain.i block=Materials/kelvin_voigt

For the creep strains to be updated properly, the simulation must also include a [LinearViscoelasticStressUpdate](/LinearViscoelasticStressUpdate.md) user object:

!listing modules/tensor_mechanics/test/tests/visco/visco_small_strain.i block=UserObjects


!syntax parameters /Materials/ComputeLinearViscoelasticStress

!syntax inputs /Materials/ComputeLinearViscoelasticStress

!syntax children /Materials/ComputeLinearViscoelasticStress
