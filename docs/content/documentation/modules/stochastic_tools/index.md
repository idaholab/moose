# Stochastic Tools
The stochastic tools module is a toolbox designed for performing stochastic analysis for MOOSE-based
applications. The following sections detail the various aspects of this module that can be
used independently or in combination to meet the needs of the application developer.

## Distributions
Distribution objects in [MOOSE] are [function](Functions/index.md)-like in that they have methods
that are called on-demand by other objects and do not maintain any state. A custom Distribution
object is created in the typical fashion, by creating a C++ class that inherits from the
Distribution base class. Three functions are required to be overridden: "pdf", "cdf", and "quantile".

The "pdf" method must return the value of the probability density function (PDF) of the
distribution. Similarly, the "cdf" method must return the value of the cumulative distribution
function (CDF). Finally, the "quantile" method must return the inverse of the CDF, which is commonly
referred to as the [quantile function](https://en.wikipedia.org/wiki/Quantile_function).

For example \ref{uniform-distribution-h} is the header for the [UniformDistribution](stochastic_tools/UniformDistribution.md), which overrides the aforementioned
methods.

!listing modules/stochastic_tools/include/distributions/UniformDistribution.h id=uniform-distribution-h caption=Header for the UniformDistribution object that includes the three required method overrides for creating a distribution.

To utilize a Distribution object within an input file, first the object must be created and secondly an object must be defined to use the distribution. Distribution objects may be
created in the input file within the `[Distributions]` block, as shown below.

!listing modules/stochastic_tools/tests/distributions/uniform.i block=Distributions

To use a distribution an object must inherit from the DistributionInterface, which provides
to methods:

* **getDistribution**<br>
This method accepts the name of an input parameter added via a call with the  
`addParam<DistributionName>` method. In general, application developers will use this method.

* **getDistributionByName**<br>
This method accepts the explicitly defined name of a distribution. In general, application
developers *will not* utilize this method.

Each of these methods return a reference to Distribution object, from which you call the
various methods on the object as discussed previously.

## Distribution Objects
The following lists the available Distribution objects within the Stochastic Tools module.

!subobjects /Distributions title=None

## Samplers
Sampler objects in [MOOSE] are designed to generate an arbitrary set of data sampled from
any number of Distribution objects.

The sampler operators by iterating over each Distribution object assigned to the sampler via the
"distributions" input parameter. From each distribution a matrix (`libMesh::DenseVector<Real>`)
is returned, such that objects using the sampler (via the SamplerInterface) will call the
"getSamples" method and receive a vector of the matrix objects, one for each distribution object.

Application developers that require a custom sampling strategy must create an object that inherits
from Sampler and overrides the "sampleDistribution" method. This method has two inputs a reference
to the current distribution and the index of that same distribution.

## Sampler Objects
The following lists the available Sampler objects within the Stochastic Tools module.

!subobjects /Samplers title=None
