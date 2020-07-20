# SquaredExponentialCovariance

!syntax description /Covariance/SquaredExponentialCovariance

## Overview

A widely used, general purpose isotropic covariance function is

!equation
k(x,x^\prime) = \sigma_f^2 \, exp \left(- \frac{r_\ell(x,x^\prime)^2}{2} \right) + \sigma_n^2 \, \delta_{x,x^\prime}.

$r_\ell(x,x^\prime)$ is a scaled distance based on the length factor $\vec{\ell}$, defined as

!equation
r_\ell(x,x^\prime) = \sqrt{ \sum_n \left( \frac{x_i - x^\prime_i}{\ell_i} \right)^2}.

## Hyperparameters

!table id=HyperpramTable caption=Hyperparameters for Squared Exponential Covariance Function
| Variable | Domain| Description |
| - | - | - |
| $\vec{\ell}$ | $\mathbb{R}_{>0}^n$ | Length factors corresponding to input parameters\* |
| $\sigma_f$ | $\mathbb{R}_{\geq 0}$ | Signal variance\* |
| $\sigma_n$ | $\mathbb{R}_{\geq 0}$ | Noise variance\* |

\*See the [Gaussian Process Trainer](GaussianProcessTrainer.md) documentation for more in depth explanation of $\vec{\ell}$, $\sigma_f$, and $\sigma_n$ hyperparameters.

## Example Input File Syntax

!! Describe and include an example of how to use the SquaredExponentialCovariance object.

!listing test/tests/surrogates/gaussian_process/GP_squared_exponential.i block=Covariance

!syntax parameters /Covariance/SquaredExponentialCovariance

!syntax inputs /Covariance/SquaredExponentialCovariance

!syntax children /Covariance/SquaredExponentialCovariance
