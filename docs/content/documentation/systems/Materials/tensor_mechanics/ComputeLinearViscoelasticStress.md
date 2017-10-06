# ComputeLinearViscoelasticStress
This computes the stress of a linear viscoelastic material using a total small strain approximation. The stress is calculated from the mechanical strain and the creep strain as:

$$
\sigma_{ij} = C_{ijkl} \left( \epsilon^{mech}_{kl} - \epsilon^{creep}_{kl} \right)
$$.

The mechanical strain must be computed with a [ComputeSmallStrain](/ComputeSmallStrain.md) material. The creep strain itself is computed by a linear viscoelastic material such as a [GeneralizedKelvinVoigtModel](/GeneralizedKelvinVoigtModel.md) or a [GeneralizedMaxwellModel](/GeneralizedMaxwellModel.md) material. The elasticity tensor $C_{ijkl}$ is also provided by the same linear viscoelastic material.

For the creep strains to be updated properly, the simulation must also include a [LinearViscoelasticStressUpdate](/LinearViscoelasticStressUpdate.md) user object.

!syntax description /Materials/ComputeLinearViscoelasticStress

!syntax parameters /Materials/ComputeLinearViscoelasticStress

!syntax inputs /Materials/ComputeLinearViscoelasticStress

!syntax children /Materials/ComputeLinearViscoelasticStress
