# GaussianProcess

!syntax description /Surrogates/GaussianProcess

The theory and use this object is provided within a discussion of the [GaussianProcessTrainer.md] training
object.

A desirable aspect of Gaussian process modeling is that in addition to returning a predicted value at the evaluation point, it can also provide a measure of uncertainty in the form of a standard deviation. To facilitate this an overloaded `evaluate()` function which sets the standard deviation by reference is provided.

!listing surrogates/GaussianProcess.C line=GaussianProcess::evaluate(const std::vector<Real> & x, Real & std_dev)


!syntax parameters /Surrogates/GaussianProcess

!syntax inputs /Surrogates/GaussianProcess

!syntax children /Surrogates/GaussianProcess
