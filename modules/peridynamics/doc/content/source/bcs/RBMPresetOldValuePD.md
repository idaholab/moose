# Rigid Body Motion Preset Old Value Boundary Condition

## Description

The `RBMPresetOldValuePD` BoundaryCondition applies Dirichlet boundary condition of old variable solution to rigid body motion (RBM) material points based on the singularity of shape tensor which is a concept from non-ordinary state-based peridynamics formulation. The rigid body motion of a material point, i.e., not fully constrained for all degrees of freedom, is equivalent to singularity of its shape tensor. This BC can be used to fix the non-fully-constrained material points hence improve the solvability of the whole system.

!syntax parameters /BCs/RBMPresetOldValuePD

!syntax inputs /BCs/RBMPresetOldValuePD

!syntax children /BCs/RBMPresetOldValuePD
