# Compute Volumetric Eigenstrain
!syntax description /Materials/ComputeVolumetricEigenstrain

## Description
This material computes the eigenstrain tensor based on a set of scalar material properties
which when summed together define the volumetric strain. The materials taken as input to this
model specify the ratio $V/V_0$, where $V$ is the current volume and $V_0$ is the initial
volume.

In models that use finite strain formulations, the volume change resulting from
this eigenstrain will exactly equal the specified volumetric strain.

##Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/volumetric_eigenstrain/volumetric_eigenstrain.i block=Materials/thermal_expansion_strain1

where the volumetric material is defined as a separate material model
!listing modules/tensor_mechanics/test/tests/volumetric_eigenstrain/volumetric_eigenstrain.i block=Materials/volumetric_change

The `eigenstrain_name` parameter value must also be set for the strain calculator, and an example parameter setting is shown below:
!listing modules/tensor_mechanics/test/tests/volumetric_eigenstrain/volumetric_eigenstrain.i block=Materials/strain

!syntax parameters /Materials/ComputeVolumetricEigenstrain

!syntax inputs /Materials/ComputeVolumetricEigenstrain

!syntax children /Materials/ComputeVolumetricEigenstrain
