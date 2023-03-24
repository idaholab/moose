# TruncatedGaussian

!syntax description /Likelihood/TruncatedGaussian

## Overview

!equation
k(x,x^\prime) = \sigma_f^2 \, exp \left(- r_\ell(x,x^\prime)^\gamma \right) + \sigma_n^2 \, \delta_{x,x^\prime},

## Example Input File Syntax

!listing test/tests/reporters/likelihoods/gaussian/main.i block=Likelihood

!syntax parameters /Likelihood/TruncatedGaussian

!syntax inputs /Likelihood/TruncatedGaussian

!syntax children /Likelihood/TruncatedGaussian
