# MFEMPeriodic

The `MFEMPeriodic` syntax is used to add periodic boundary conditions to an MFEM mesh.
The user can specify up to n translation vectors of dimension n for an n-dimensional problem. These are passed to the MFEM mesh,
which identifies pairs of nodes which are separated by this translation vector, and removes
one from the system.

More information can be found on the [MFEM website](https://mfem.org/howto/periodic-boundaries/).