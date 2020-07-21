# LoadCovarianceDataAction

This action operates on existing `GaussianProcess` objects contained within the `[Surrogates]` block.
If the model provides a filename (as shown below), a `[Covariance]` object equivalent to the function used in the training phase is reconstructed for use in model evaluation.

## Example Input File Syntax

In the training input file we setup a GaussianProcessTrainer, with a SquaredExponential covariance function.

!listing test/tests/surrogates/gaussian_process/GP_squared_exponential_training.i block=Trainers Covariance

In the surrogate input file, the GaussianProcess surrogate recreates the covariance function used in training and links to it.

!listing test/tests/surrogates/gaussian_process/GP_squared_exponential_testing.i block=Surrogates
