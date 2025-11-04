# PINSFEMaterial

!syntax description /Materials/PINSFEMaterial

This material is the porous media version of [INSFEMaterial.md]. It declares and computes
the same material properties. It additionally can set constant user-input values for the following
properties:

- the inertia resistance coefficient
- the viscous resistance coefficient
- the porous media heat transfer coefficient between the solid and fluid phase
- the porous media wetted area


Non-constant properties can be defined for these quantities by creating a derived class of the
[PINSFEMaterial.md] and implementing correlations for the properties there.

!syntax parameters /Materials/PINSFEMaterial

!syntax inputs /Materials/PINSFEMaterial

!syntax children /Materials/PINSFEMaterial
