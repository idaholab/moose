# Standardizer

!if! function=hasCapability('libtorch')

## Overview

The Standardizer centers and scales tensor data (subtracting the mean and dividing by the
standard deviation of each column) and reverses the operation. It is used throughout the
Gaussian Process and libtorch neural network utilities, e.g. [GaussianProcess.md], to put
input parameters and output data into a normalized form before training or evaluation, then
map predictions back into their original units.

`computeSet` computes and stores the per-column mean and standard deviation from a reference
tensor, and `getStandardized`/`getDestandardized` apply and reverse that
transform:

!listing stochastic_tools/src/utils/GaussianProcess.C
         start=GaussianProcess::standardizeParameters
         end=}

`getScaled`/`getDescaled` instead only scale by the standard deviation, without shifting by
the mean, which is useful for quantities such as variances or derivatives where centering
does not apply.

### Recovering from a checkpoint

A Standardizer that has not yet had `computeSet`/`set` called has no defined mean or
standard deviation. Its `dataStore`/`dataLoad` specializations store a flag recording
whether the moments are defined before storing them, so that an empty Standardizer
correctly round-trips through a checkpoint without one being fabricated on load:

!listing stochastic_tools/src/utils/Standardizer.C
         start=const bool defined = _mean.defined();
         end=return;
         include-end=True

!if-end!

!else
!include libtorch/libtorch_warning.md
