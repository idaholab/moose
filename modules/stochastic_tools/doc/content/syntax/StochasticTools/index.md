# StochasticTools System

The `StochasticTools` block is provides parameters that provide convenience for running stochastic
simulations. For example, if the block is present it will automatically create the
necessary objects for running a parameter study style stochastic analysis. This is where the
main input file does not perform a solve, but simply spawns other simulations. Please
refer to [examples/parameter_study.md] for an example.

!listing examples/parameter_study/main.i block=StochasticTools

!syntax parameters /StochasticTools

!syntax list /StochasticTools objects=True actions=False subsystems=False

!syntax list /StochasticTools objects=False actions=False subsystems=True

!syntax list /StochasticTools objects=False actions=True subsystems=False
