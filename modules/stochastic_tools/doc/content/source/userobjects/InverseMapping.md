# InverseMapping

!syntax description /UserObjects/InverseMapping

## Overview

This object is responsible for mapping the solution coordinates in a low-dimensional
latent space into high-dimensional MOOSE solution vectors. For this, we need a mapping object which can be
defined using the [!param](/UserObjects/InverseMapping/mapping) input parameter.
The method populates `AuxVariables` or `Variables` with the approximated high-order solution vectors. The user can specify the
these using [!param](/UserObjects/InverseMapping/variable_to_fill). At the same time, to be able to identify
the [!ac](DOF) connections between the basis functions in the mapping and the variable,
this object expects the user to input the real (nonlinear) variable name as well using the
[!param](/UserObjects/InverseMapping/variable_to_reconstruct) parameter. This object can function in two distinct ways:

1. If [!param](/UserObjects/InverseMapping/surrogate) is supplied, we use a surrogate model to
   generate the coordinates in the latent space (e.g. [PolynomialRegressionSurrogate.md]). In this case
   the [!param](/UserObjects/InverseMapping/parameters) input parameter refers to the model parameters
   (created using a [Sampler](Samplers/index.md) or manually). One can define different surrogates for different variables.
2. If no surrogate is supplied, we assume that the user supplies the coordinates in the low-dimensional space using
   the [!param](/UserObjects/InverseMapping/parameters) input parameter.


## Example Input File Syntax

!listing test/tests/userobjects/inverse_mapping/inverse_map.i block=UserObjects

## Syntax

!syntax parameters /UserObjects/InverseMapping

!syntax inputs /UserObjects/InverseMapping

!syntax children /UserObjects/InverseMapping
