# Optimization System

Any input file in [MOOSE] needs to include a [Mesh](Mesh/index.md),
and [Variables](syntax/Variables/index.md) block.  However, the [optimize](Optimize.md) executioner on the optimization main app uses [PETSc TAO](https://www.mcs.anl.gov/petsc/documentation/taosolvertable.html) to solve an optimization problem and does not solve a normal [MOOSE] finite element based nonlinear system of equations.  So the [Optimization](Optimization/index.md) builds a minimal model to satisfy these requirements.
If a mesh is already provided in the main app, then a minimal mesh is not created.

!listing test/tests/optimizationreporter/point_loads/main.i block=Optimization

!syntax parameters /Optimization

!syntax list /Optimization objects=True actions=False subsystems=False

!syntax list /Optimization objects=False actions=False subsystems=True

!syntax list /Optimization objects=False actions=True subsystems=False
