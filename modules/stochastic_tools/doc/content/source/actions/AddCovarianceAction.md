# AddCovarianceAction

This action adds `CovarianceFunctionBase` [asds](Covariance/index.md) objects contained within the `[Covariance]` block. For example,
the following block adds a [SquaredExponentialCovariance.md] covariance function.

!listing test/tests/surrogates/gaussian_process/GP_squared_exponential_training.i block=Covariance

!if! function=hasCapability('libtorch')

!syntax parameters /Covariance/AddCovarianceAction

!if-end!

!else
!include libtorch/libtorch_warning.md
