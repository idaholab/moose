# MorrisReporter

!syntax description /Reporters/MorrisReporter

## Overview

This object computes the Morris sensitivity quantities following the procedure defined by
[!cite](saltelli2008global). The MorrisReporter object only operates on result data
generated from Morris-based sampling strategy, which is provided by the [MorrisSampler.md].

## Elementary Effects

Much of the following information is taken directly from the [GSA module](https://gsa-module.readthedocs.io/en/stable/implementation/morris_screening_method.html), see this documentation for more details.

Given a model with $D$ parameters defined by the vector $\vec{x}$ and a model with response function $f(\vec{x})$ the elementary effect for a given parameter $k$ is defined as [!cite](morris1991factorial):

!equation
EE_k = \frac{f(\vec{x} + \vec{\Delta_d}) - f(\vec{x})}{\Delta_d}\,, k=1,...,D ,

where $\Delta_k$ is a perturbation of parameter $x_k$ and $\vec{\Delta_k}$ is a vector of length $D$ of all zeros except the $k$th entry equal to $\Delta_k$. Given a one-at-a-time (OAT) sampling strategy, detailed in [MorrisSampler.md], with $N$ replicates, or trajectories, there will be a total of $N\times D$ elementary effects. The purpose of computing these elementary effects is to evaluate a local sensitivity over these random trajectories, the global sensitivity can then be gleaned by accumulating these local effects. As such, this reporter computes the following statistics of this elementary effect matrix:

!equation
\mu_d = \frac{1}{N}\sum_{i=1}^{N}EE_{i,d},

!equation
\mu_d^{*} = \frac{1}{N}\sum_{i=1}^{N}|EE_{i,d}|,

!equation
\sigma_d = \sqrt{\frac{1}{N-1}\sum_{i=1}^{N}\left(EE_{i,d} - \mu_d\right)^2} .

The $\mu^{*}$ statistic is typically preferred over $\mu$ because it is agnostic to negative effects that could cancel out positive ones when sampling. These statistics can be interperated as the effect of the parameters on the response in the folloing ways:

1. $\mu^{*} \approx 0, \sigma \approx 0$: parameter has no influential impact on the response.
1. $\mu^{*} >> 0$: parameter has a significant impact on the response.
1. $\sigma >> 0$: parameter has nonlinear or interactive effects.
1. $\mu^{*} >> 0, \sigma \approx 0$: parameter is additive or linear
1. $\mu^{*} \approx 0, \sigma >> 0$: parameter has a negligible aggregate effect on the response while nonlinear perturbations (perturbing in more than one direction) can be significant.

This reporter can also compute confidence intervals using percentile bootstrapping. This works by randomly sampling replicates, or trajectories, re-computing the statistics, sorting them, then choosing the requested percentile.

### Trajectory Design

Computing elementary effects from the trajectory design is rather simple. For a single trajectory matrix ($\mathbf{x_{i}}$) and the corresponding response vector ($\vec{y_i}$), the elementary effects ($\vec{EE_{i}}$) can be computed as:

!equation
\vec{EE_{i}} = \delta\mathbf{x_i}^{-1}\delta\vec{y_i}\,, i=1,...,N,

where

!equation
(\delta\mathbf{x_i})_{j,d} = (\mathbf{x_i})_{j+1,d} - (\mathbf{x_i})_{j,d},\quad  (\delta\vec{y_i})_{j,d} = (\vec{y_i})_{j+1,d} - (\vec{y_i})_{j,d},\quad j,d=1,...,D

Taking the first trajectory in [this sampling](test/tests/samplers/morris/gold/morris_out_data_0000.csv):

!equation
\mathbf{x_1} = \begin{bmatrix}
0.4 & 0.0 & 0.2 \\
0.4 & 0.6 & 0.2 \\
0.4 & 0.6 & 0.8 \\
1.0 & 0.6 & 0.8
\end{bmatrix} , \quad
\delta\mathbf{x_i} = \begin{bmatrix}
0 & 0.6 & 0 \\
0 & 0 & 0.6 \\
0.6 & 0 & 0
\end{bmatrix},

!equation
\vec{y_1} = \begin{bmatrix}
y_1 \\
y_2 \\
y_3 \\
y_4
\end{bmatrix} , \quad
\delta\vec{y_1} = \begin{bmatrix}
y_2 - y_1 \\
y_3 - y_2 \\
y_4 - y_3
\end{bmatrix},

!equation
\vec{EE_1} = \begin{bmatrix}
(y_2 - y_1)/0.6 \\
(y_3 - y_2)/0.6 \\
(y_4 - y_3)/0.6
\end{bmatrix}.

## Example Input Syntax

The following example computes the elementary effect statistics along with 10% and 90% confidence points for two scalar quantities and one vector quantity:

!listing reporters/morris/morris_main.i block=Reporters

The resulting output is a [json file](JSONOutput.md) with the statistics and their confidence intervals:

!listing reporters/morris/gold/morris_main_out.json language=json

We see that for each vector we have three quantities: `mu` for $\mu$, `mu_star` for $\mu^{*}$, `sigma` for $\sigma$. Each has a pair contianing a vector and a vector of vectors of the response value type. The first is the computed value for each parameter. The second has an entry for these values for each confidence point.

!syntax parameters /Reporters/MorrisReporter

!syntax inputs /Reporters/MorrisReporter

!syntax children /Reporters/MorrisReporter
