# GaussianProcessData

The GaussianProcessData object simply coverts a the covariance function hyperparameters into a VectorPostprocessor for post regression analysis of hyperparameters.

## Example Syntax

!listing modules/stochastic_tools/test/tests/surrogates/gaussian_process/GP_squared_exponential_tuned.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/GaussianProcessData

!syntax inputs /VectorPostprocessors/GaussianProcessData

!syntax children /VectorPostprocessors/GaussianProcessData
