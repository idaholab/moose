# MeshAlignmentVariableTransferMaterial

This material is used to create an AD material property for a variable from a
coupled subdomain/boundary via [MeshAlignment.md].

It currently assumes:

- The variable is from the nonlinear system (is a solution variable).
- The variable uses 1st-order Lagrange finite elements.

!syntax parameters /Materials/MeshAlignmentVariableTransferMaterial

!syntax inputs /Materials/MeshAlignmentVariableTransferMaterial

!syntax children /Materials/MeshAlignmentVariableTransferMaterial
