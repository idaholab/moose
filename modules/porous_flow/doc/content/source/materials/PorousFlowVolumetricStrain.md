# PorousFlowVolumetricStrain

This Material computes volumetric strain and volumetric strain rate.  It requires a TensorMechanics strain calculator, which will usually be [ComputeSmallStrain](ComputeSmallStrain.md), to compute the Material property `total_strain`.  It then calculates

- `PorousFlow_total_volumetric_strain_qp` which is $\sum_{i}\epsilon_{ii}$, where $\epsilon$ is `total_strain`
- `PorousFlow_volumetric_strain_rate_qp` which is $\sum_{i}(\epsilon_{ii} - \epsilon_{ii}^{\mathrm{old}})/\mathrm{d}t$.

It also calculates derivatives of these with respect to the PorousFlow variables.

!alert note
It is possible to supply a `base_name` if your TensorMechanics strain calculator also has a `base_name`.

!alert note
The derivatives calculated assume Cartesian small strain.  Therefore, if you use a finite-strain calculator, or if you're using other coordinate systems, such as RZ coordinates, the Jacobian won't be exact, which will lead to slightly poorer convergence

This Material provides the time-derivative of the volumetric strain to the [PorousFlowMassVolumetricExpansion](PorousFlowMassVolumetricExpansion.md) and [PorousFlowHeatVolumetricExpansion](PorousFlowHeatVolumetricExpansion.md).  Related information can be found [here](porous_flow/time_derivative.md).


!syntax parameters /Materials/PorousFlowVolumetricStrain

!syntax inputs /Materials/PorousFlowVolumetricStrain

!syntax children /Materials/PorousFlowVolumetricStrain
