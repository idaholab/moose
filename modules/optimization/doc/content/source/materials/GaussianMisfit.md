# GaussianMisfit

!syntax description /Materials/GaussianMisfit

## Overview

The `GaussianMisfit` material is a specialized material that calculates a misfit value and its gradient with respect to a simulation variable, using a Gaussian function to weight the contribution of measurement points



Measurement data:

- Measurement values $\mathbf{u}_m = \{u_{m,1}, u_{m,2}, \ldots, u_{m,n}\}$
- Measurement Locations: $\mathbf{p}_m = \{\mathbf{p}_{m,1}, \mathbf{p}_{m,2}, \ldots, \mathbf{p}_{m,n}\}$, where $\mathbf{p}_{m,i} = (x_{m,i}, y_{m,i}, z_{m,i})$ for 3D data

- Simulation variable: $u(\mathbf{x})$
- Gaussian beam width: $w$
- Simulation point: $\mathbf{x}$

The `misfit` at a given simulation point $\mathbf{x}$ is calculated as:

$m(\mathbf{x}) = \sum_{i=1}^{n}  \left( u_{m,i} g(\mathbf{p}_{m,i}, \mathbf{x}) - u(\mathbf{x}) g(\mathbf{p}_{m,i}, \mathbf{x}) \right)^2,$

where:

- $g(\mathbf{p}_{m,i}, \mathbf{x})$ is the Gaussian weighting function, defined as:

$g(\mathbf{p}_{m,i}, \mathbf{x}) = \exp\left( -\frac{2 \|\mathbf{p}_{m,i} - \mathbf{x}\|^2}{w^2} \right),$

and the `misfit gradient` with respect to the simulation variable $u(\mathbf{x})$ is:

$\frac{\partial m(\mathbf{x})}{\partial u(\mathbf{x})} = -2 \sum_{i=1}^{n} g(\mathbf{p}_{m,i}, \mathbf{x}) \left( u_{m,i} g(\mathbf{p}_{m,i}, \mathbf{x}) - u(\mathbf{x}) g(\mathbf{p}_{m,i}, \mathbf{x}) \right).$

These equations represent the misfit value and its gradient computed by the `GaussianMisfit` material at each quadrature point in the simulation. The misfit value quantifies the discrepancy between the measured data and the simulation variable, while the misfit gradient provides the sensitivity of the misfit with respect to changes in the simulation variable.

## Example Input File Syntax

This material can be used to optimize a multimaterial topology problem, in which the
constraints are not only based on volume but also on an additional cost function.

!listing test/tests/optimizationreporter/gaussian_misfit/forward_and_adjoint.i block=Materials/beam

!syntax parameters /Materials/GaussianMisfit

!syntax inputs /Materials/GaussianMisfit

!syntax children /Materials/GaussianMisfit
