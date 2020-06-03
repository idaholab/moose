# Singular Shape Tensor Eliminator UserObject

## Description

The `SingularShapeTensorEliminatorUserObjectPD` UserObject is used to eliminate the occurrence of singular shape tensor in a peridynamic simulation. This userobject is executed at the time step end with updated bond status from aux kernel. Shape tensor is formed using neighboring bonds (potentially ghosted bonds from neighboring processor(s)) for each node of a bond. A bond with at least one singular associated shape tensor is treated as broken as if determined by the bond-based failure criteria. This process is repeated until no further singular shape tensor is detected.

Occurrence of singular shape tensor usually happens in material failure problems modeled using formulations based on the peridynamic deformation gradient, such as the correspondence models and the ordinary state-based generalized plane strain model.

!syntax parameters /UserObjects/SingularShapeTensorEliminatorUserObjectPD

!syntax inputs /UserObjects/SingularShapeTensorEliminatorUserObjectPD

!syntax children /UserObjects/SingularShapeTensorEliminatorUserObjectPD
