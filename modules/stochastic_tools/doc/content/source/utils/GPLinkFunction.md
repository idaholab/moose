# GPLinkFunction

## Overview

`GPLinkFunction` provides output warping (link functions) for [GaussianProcessTrainer.md]
that enforce inequality constraints on GP predictions. A link function $g$ maps
the physical output $y$ to a latent (unconstrained) variable $z = g(y)$. The GP
is trained in latent $z$-space, and predictions are mapped back to physical space
via the inverse $y = g^{-1}(z)$.

This approach is sometimes called *warped Gaussian processes*
[!cite](snelson2004warped) and provides an exact constraint: every prediction
satisfies the bound by construction.

## Available Link Functions

### Identity (default)

No transformation is applied. The GP trains and predicts directly in physical space.

!equation
g(y) = y, \quad g^{-1}(z) = z

Use this (or omit `link_function`) for unconstrained problems.

### Log Link

Enforces a strict lower bound: $y > \ell$.

!equation
g(y) = \ln(y - \ell), \quad g^{-1}(z) = e^{z} + \ell

where $\ell$ is `link_lower_bound` (default 0, giving positivity constraint).

The Jacobian correction $\ln |g'(y)| = -\ln(y - \ell)$ is included in the
negative log-marginal likelihood when hyperparameter tuning is active.

### Logit Link

Enforces both a lower and upper bound: $\ell < y < u$.

!equation
g(y) = \ln\!\left(\frac{y - \ell}{u - y}\right), \quad
g^{-1}(z) = \ell + \frac{u - \ell}{1 + e^{-z}}

where $\ell$ is `link_lower_bound` and $u$ is `link_upper_bound`. Requires
$u > \ell$.

## Uncertainty Propagation

When a link function is active, GP posterior variance $\sigma_z^2$ in latent space
is mapped to physical space via the delta method:

!equation
\sigma_y \approx \left| (g^{-1})'(\mu_z) \right| \, \sigma_z

This is an approximation that is accurate when $\sigma_z$ is small relative to
the curvature of $g^{-1}$.

## Interaction with Standardization

The link transform is applied to training data *before* standardization.
The training pipeline in [GaussianProcessTrainer.md] proceeds as:

1. Collect physical responses $y_i$.
2. Apply link: $z_i = g(y_i)$.
3. Standardize (center/scale) the $z_i$.
4. Train GP in standardized $z$-space.

At prediction time, the GP mean and standard deviation are de-standardized to
$z$-space and then the inverse link is applied.

## Usage

Link functions are configured via parameters on [GaussianProcessTrainer.md]:

!listing stochastic_tools/test/tests/surrogates/gaussian_process/GP_log_link.i
         block=Trainers

!listing stochastic_tools/test/tests/surrogates/gaussian_process/GP_logit_link.i
         block=Trainers
