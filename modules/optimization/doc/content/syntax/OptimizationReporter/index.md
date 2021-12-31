# OptimizationReporter

The `OptimizationReporter` block sets up a reporter used for communicating data between the optimization executioner
and the transfers to the sub-app Forward and adjoint solves.  Only one `OptimizationReporter` is allowed per main-app. 
Please refer to [examples/force_bc_inversion.md] for an example.

!listing test/tests/optimizationreporter/objective_gradient_minimize/point_loads/master.i
         block=OptimizationReporter

!syntax parameters /OptimizationReporter

!syntax list /OptimizationReporter objects=True actions=False subsystems=False groups=isopodApp

!syntax list /OptimizationReporter objects=False actions=True subsystems=False
