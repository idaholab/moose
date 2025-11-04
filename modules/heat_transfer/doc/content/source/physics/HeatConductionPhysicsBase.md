# HeatConductionPhysicsBase

This class serves as a base class for `Physics` discretizing the heat conduction equations.
It notably:

- defines shared parameters
- sets up the initial condition for the temperature
- defines a default preconditioning using HYPRE's boomeramg
