# Gaussian

!syntax description /Likelihood/Gaussian

## Overview

!equation
k(x,x^\prime) = \sigma_f^2 \, exp \left(- r_\ell(x,x^\prime)^\gamma \right) + \sigma_n^2 \, \delta_{x,x^\prime},

## Example Input File Syntax

!listing test/tests/reporters/likelihoods/gaussian/main.i block=Likelihood

!syntax parameters /Likelihood/Gaussian

!syntax inputs /Likelihood/Gaussian

!syntax children /Likelihood/Gaussian
