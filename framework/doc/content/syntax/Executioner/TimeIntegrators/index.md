# Multiple Time Integrators

The time integrator system is described in [TimeIntegrator/index.md]. Multiple
time integrators in a single input are supported using
`Executioner/TimeIntegrators` syntax as illustrated below:

!listing test/tests/time_integrators/multiple-integrators/test.i block=Executioner

where each individual time integrator has specific `variables` assigned to
it.

!syntax list /Executioner/TimeIntegrators objects=True actions=False subsystems=False

!syntax list /Executioner/TimeIntegrators objects=False actions=False subsystems=True

!syntax list /Executioner/TimeIntegrators objects=False actions=True subsystems=False

!syntax parameters /Executioner/TimeIntegrators
