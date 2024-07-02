# ExponentialCovariance

!syntax description /Covariance/ExponentialCovariance

## Overview

A simple exponential covariance function can be constructed as

!equation
k(x,x^\prime) = \sigma_f^2 \, exp \left(- r_\ell(x,x^\prime)^\gamma \right) + \sigma_n^2 \, \delta_{x,x^\prime},

which is valid for $0 < \gamma \leq 2$. $r_\ell(x,x^\prime)$ is a scaled distance based on the length factor $\vec{\ell}$, defined as

!equation
r_\ell(x,x^\prime) = \sqrt{ \sum_n \left( \frac{x_i - x^\prime_i}{\ell_i} \right)^2}.

When $\gamma = 2$ is equivalent to [](SquaredExponentialCovariance.md), save a factor of $2$ (which can be absorbed into $\vec{\ell}$).

## Hyperparameters

!table id=HyperpramTable caption=Hyperparameters for Exponential Covariance Function
| Variable | Domain| Description |
| - | - | - |
| $\vec{\ell}$ | $\mathbb{R}_{> 0}^n$ | Length factors corresponding to input parameters\* |
| $\sigma_f$ | $\mathbb{R}_{\geq 0}$ | Signal variance\* |
| $\sigma_n$ | $\mathbb{R}_{\geq 0}$ | Noise variance\* |
| $\gamma$ | $(0,2]$ | Exponential factor |

\*See the [Gaussian Process Trainer](GaussianProcessTrainer.md) documentation for more in depth explanation of $\vec{\ell}$, $\sigma_f$, and $\sigma_n$ hyperparameters.

## Example Input File Syntax

!listing test/tests/surrogates/gaussian_process/GP_exponential.i block=Covariance

!syntax parameters /Covariance/ExponentialCovariance

!syntax inputs /Covariance/ExponentialCovariance

!syntax children /Covariance/ExponentialCovariance
