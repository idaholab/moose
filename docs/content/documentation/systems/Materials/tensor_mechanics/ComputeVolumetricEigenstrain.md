# ComputeVolumetricEigenstrain
!syntax description /Materials/ComputeVolumetricEigenstrain

This material computes the eigenstrain tensor based on a set of scalar material properties
which when summed together define the volumetric strain. The materials taken as input to this
model specify the ratio $V/V_0$, where $V$ is the current volume and $V_0$ is the initial
volume.

In models that use finite strain
formulations, the volume change resulting from this eigenstrain will exactly equal the
specified volumetric strain.

!syntax parameters /Materials/ComputeVolumetricEigenstrain

!syntax inputs /Materials/ComputeVolumetricEigenstrain

!syntax children /Materials/ComputeVolumetricEigenstrain
