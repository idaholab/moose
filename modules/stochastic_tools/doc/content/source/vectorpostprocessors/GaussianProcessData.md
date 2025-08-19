# GaussianProcessData

The GaussianProcessData object simply coverts a the covariance function hyperparameters into a VectorPostprocessor for post regression analysis of hyperparameters.

## Example Syntax

!listing modules/stochastic_tools/test/tests/surrogates/gaussian_process/GP_squared_exponential_tuned_adam.i block=VectorPostprocessors

!if! function=hasCapability('libtorch')
!syntax parameters /VectorPostprocessors/GaussianProcessData

!syntax inputs /VectorPostprocessors/GaussianProcessData

!syntax children /VectorPostprocessors/GaussianProcessData

!if-end!

!else
!include libtorch/libtorch_warning.md
