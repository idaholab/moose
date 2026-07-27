# FESpaceHierarchies

`FESpaceHierarchies` syntax is used to create MFEM finite element space hierarchies from existing
MFEM finite element spaces. Each hierarchy defines a sequence of h- and/or p-refined levels that can
be used by MFEM variables and multigrid solvers.

The available hierarchy type is [MFEMFESpaceHierarchy.md]. Hierarchy sub-blocks are added to MFEM
problems by [AddMFEMFESpaceHierarchyAction.md].
