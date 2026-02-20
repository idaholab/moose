# MFEMPeriodic

The `MFEMPeriodic` syntax is used to add periodic boundary conditions to an MFEM mesh.
The user must specify some translation vectors: either 2 vectors of dimension 2 for a 2D
problem, or 3 vectors of dimension 3 for a 3D problem. These are passed to the MFEM mesh,
which identifies pairs of nodes which are separated by this translation vector, and removes
one from the system.

More information can be found on the [MFEM website](https://mfem.org/howto/periodic-boundaries/).