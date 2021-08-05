# NearestPointSurrogate

!syntax description /Surrogates/NearestPointSurrogate

## Overview

The NearestPointSurrogate is arguably the simplest surrogate model that can be implemented. First, the surrogate is trained by collecting sample points and results from the full model. The surrogate's `evaluate` function then takes in a point and finds the training point that has the minimum Euclidean distance:

!equation
\vec{x}_{\min} = \argmin_{\vec{x}\in\mathbf{X}}\left(\sum_{d=1}^{D}(x_d - x_{\mathrm{in},d})^2\right)

where $\vec{x}_{\min}$ is the training point closest to the input point $\vec{x}_{\mathrm{in}}$, $\mathbf{X}$ is the array of sample points from the training data, and $D$ is the number of columns in the sampler. The `evaluate` function then returns the full model result associated with $\vec{x}_{\min}$.

## Example Input File Syntax

See [NearestPointTrainer.md] for details regarding the training phase of the surrogate. The surrogate model first loads the data from the training run:

!listing nearest_point/evaluate.i block=Surrogates

A sampler is created to create sample points:

!listing nearest_point/evaluate.i block=Samplers

A reporter is then used to evaluate the surrogate model with the points taken from the sampler:

!listing nearest_point/evaluate.i block=Reporters

!syntax parameters /Surrogates/NearestPointSurrogate

!syntax inputs /Surrogates/NearestPointSurrogate

!syntax children /Surrogates/NearestPointSurrogate
