# MaternHalfIntCovariance

!syntax description /Covariance/MaternHalfIntCovariance

## Overview

A special case of the Matern class of covariance functions, in which the $\nu$ hyperparameter takes on a half integer value. Substituting a positive integer $p$ for $\nu$ using $\nu = p + 1/2$ the covariance function is given as

!equation
k(x,x^\prime) = \sigma_f^2 \, exp \left(- \sqrt{2p+1} r_\ell(x,x^\prime) \right) \frac{p!}{(2p)!} \sum_{i=0}^p \left[ \frac{(p+i)!}{i! (2p-i)!} \left( 2 \sqrt{2p+1} r_\ell(x,x^\prime) \right)^{p-i}  \right]   + \sigma_n^2 \, \delta_{x,x^\prime}.

$r_\ell(x,x^\prime)$ is a scaled distance based on the length factor $\vec{\ell}$, defined as

!equation
r_\ell(x,x^\prime) = \sqrt{ \sum_n \left( \frac{x_i - x^\prime_i}{\ell_i} \right)^2}.

## Hyperparameters

!table id=HyperpramTable caption=Hyperparameters for Matern Covariance Function
| Variable | Domain| Description |
| - | - | - |
| $\vec{\ell}$ | $\mathbb{R}_{>0}^n$ | Length factors corresponding to input parameters\* |
| $\sigma_f$ | $\mathbb{R}_{\geq 0}$ | Signal variance\* |
| $\sigma_n$ | $\mathbb{R}_{\geq 0}$ | Noise variance\* |
| $p$ | $\mathbb{N}$ |  |

\*See the [Gaussian Process Trainer](GaussianProcessTrainer.md) documentation for more in depth explanation of $\vec{\ell}$, $\sigma_f$, and $\sigma_n$ hyperparameters.

## Example Input File Syntax

!listing test/tests/surrogates/gaussian_process/GP_Matern_half_int.i block=Covariance

!syntax parameters /Covariance/MaternHalfIntCovariance

!syntax inputs /Covariance/MaternHalfIntCovariance

!syntax children /Covariance/MaternHalfIntCovariance
