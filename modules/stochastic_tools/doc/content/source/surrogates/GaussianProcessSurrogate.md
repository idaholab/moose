# GaussianProcessSurrogate

!syntax description /Surrogates/GaussianProcessSurrogate

The theory and use this object is provided within a discussion of the [GaussianProcessTrainer.md] training
object.

A desirable aspect of Gaussian process modeling is that in addition to returning a predicted value at the evaluation point, it can also provide a measure of uncertainty in the form of a standard deviation. To facilitate this an overloaded `evaluate()` function which sets the standard deviation by reference is provided.

!listing surrogates/GaussianProcessSurrogate.C line=GaussianProcessSurrogate::evaluate(const std::vector<Real> & x, Real & std_dev)

## Evaluation of multi-output Gaussian Processes

For Gaussian Processes that predict multiple outputs the user can evaluate the
the mean and standard deviation estimates using the following function:

!listing surrogates/GaussianProcessSurrogate.C re=void\sGaussianProcessSurrogate::evaluate\(const std::vector<Real> & x,\s*?std::vector<Real> & y,.*?\) const

!syntax parameters /Surrogates/GaussianProcessSurrogate

!syntax inputs /Surrogates/GaussianProcessSurrogate

!syntax children /Surrogates/GaussianProcessSurrogate
