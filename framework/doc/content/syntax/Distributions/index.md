# Distributions System

Distribution objects in [MOOSE] are [function](Functions/index.md)-like in that they have methods
that are called on-demand by other objects and do not maintain any state. A custom Distribution
object is created in the typical fashion, by creating a C++ class that inherits from the
Distribution base class. Three functions are required to be overridden: "pdf", "cdf", and "quantile".

The base class for the distribution system (`Distribution`) is included in MOOSE; however,
the system was designed to be used with the stochastic tools module. This module includes
implementations of many of the common distributions.

!syntax list /Distributions objects=True actions=False subsystems=False

!syntax list /Distributions objects=False actions=False subsystems=True

!syntax list /Distributions objects=False actions=True subsystems=False
