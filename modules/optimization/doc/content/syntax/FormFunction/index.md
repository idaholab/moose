# FormFunction

The `FormFunction` block sets up a reporter used for communicating data between the optimization executioner
and the transfers to the sub-app Forward and adjoint solves.  Only one `FormFunction` is allowed per main-app. 
Please refer to [examples/force_bc_inversion.md] for an example.

!listing test/tests/formfunction/objective_gradient_minimize/point_loads/master.i
         block=FormFunction

!syntax parameters /FormFunction

!syntax list /FormFunction objects=True actions=False subsystems=False groups=isopodApp

!syntax list /FormFunction objects=False actions=True subsystems=False
