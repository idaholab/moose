# Sampler

The Sampler class is an abstract base class for creating objects for generating a matrix of
random numbers; the matrix is an arbitrary size that must be defined by the child object
by calling the `setNumberOfRows` and `setNumberOfCols` methods within the constructor of the child
class. The other requirement is to override the pure virtual `computeSample` method. This method
has global row and column index as input and must output a single value for populating the
sample data matrix.

Support for Sampler objects within the framework is minimal, please refer to the
[Stochastic Tools](modules/stochastic_tools/index.md optional=True) documentation for more
information and example use.

!syntax list /Samplers objects=True actions=False subsystems=False heading=Available Sampler Objects
