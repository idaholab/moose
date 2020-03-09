# Surrogates System

The Stochastic Tools module contains a system for training and running surrogate models. These
objects inherit from the `SurrogateModel` class and are added to a simulation using the
`[Surrogates]` block in the input file. For example, the following input file snippet adds a
[PolynomialChaos.md] surrogate model for training. Please refer to the documentation on the
individual models for more details.

!listing poly_chaos/master_2d_mc.i block=Surrogates

!syntax list /Surrogates objects=True actions=False subsystems=False

!syntax list /Surrogates objects=False actions=False subsystems=True

!syntax list /Surrogates objects=False actions=True subsystems=False
