# SingularShapeTensorEliminatorUserObjectPD

## Description

The `SingularShapeTensorEliminatorUserObjectPD` UserObject is used to eliminate the occurrence of singular shape tensors in peridynamic simulations. This UserObject is executed at end of a timestep, using updated bond status provided by an AuxKernel. The shape tensor is formed using neighboring bonds (including potentially ghosted bonds from neighboring processor(s)) for each node connected to a bond. Any bond with at least one singular associated shape tensor is treated as being broken, just as it would be if determined by a bond-based failure criterion. Because breaking bonds can have make other shape tensors singular, this can have a cascading effect, so this process is repeated until no further singular shape tensors are detected.

Occurrence of singular shape tensor usually happens in material failure problems modeled using formulations based on the peridynamic deformation gradient, such as the correspondence models and the ordinary state-based generalized plane strain model.

!syntax parameters /UserObjects/SingularShapeTensorEliminatorUserObjectPD

!syntax inputs /UserObjects/SingularShapeTensorEliminatorUserObjectPD

!syntax children /UserObjects/SingularShapeTensorEliminatorUserObjectPD
