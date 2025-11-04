# MisfitReporterOffsetFunctionMaterial

!syntax description /Materials/MisfitReporterOffsetFunctionMaterial

## Overview

The `MisfitReporterOffsetFunctionMaterial` material computes the misfit and its gradient with respect to a simulation variable , using a user-defined offset function to weight the contribution of measurement points. This material is particularly useful for inverse optimization problems where you want to match simulation results to experimental or observed data.  The material name given to the misfit is [!param](/Materials/MisfitReporterOffsetFunctionMaterial/property_name) and the misfit gradient material uses the naming convention [!param](/Materials/MisfitReporterOffsetFunctionMaterial/property_name)+`_gradient`.

### Measurement data:

Measurement values are provide by the reporter [!param](/Materials/MisfitReporterOffsetFunctionMaterial/measurement_value_name) and the corresponding measurement locations are given by a reporter containing either a vector of points [!param](/Materials/MisfitReporterOffsetFunctionMaterial/point_name) or three separate reporters each containing the x, y and z coordinates respectively of the points using [!param](/Materials/MisfitReporterOffsetFunctionMaterial/x_coord_name), [!param](/Materials/MisfitReporterOffsetFunctionMaterial/y_coord_name), and [!param](/Materials/MisfitReporterOffsetFunctionMaterial/z_coord_name).

- Measurement values: $\mathbf{u}_m = \{u_{m,1}, u_{m,2}, \ldots, u_{m,n}\}$
- Measurement locations: $\mathbf{p}_m = \{\mathbf{p}_{m,1}, \mathbf{p}_{m,2}, \ldots, \mathbf{p}_{m,n}\}$, where $\mathbf{p}_{m,i} = (x_{m,i}, y_{m,i}, z_{m,i})$ for 3D data
- Simulation variable: $u(\mathbf{x})$
- Offset function: $g(\mathbf{p}, \mathbf{x}; \mathbf{\theta})$, where $\mathbf{\theta}$ are the parameters defining the offset function
- Simulation point: $\mathbf{x}$

### Misfit and gradient calculation:

The `misfit` at a given simulation point $\mathbf{x}$ is calculated as:

$m(\mathbf{x}) = \sum_{i=1}^{n} \left( u_{m,i} g(\mathbf{p}_{m,i}, \mathbf{x}; \mathbf{\theta}) - u(\mathbf{x}) g(\mathbf{p}_{m,i}, \mathbf{x}; \mathbf{\theta}) \right)^2,$

and the `misfit_gradient` with respect to the simulation variable $u(\mathbf{x})$ is:

$\frac{\partial m(\mathbf{x})}{\partial u(\mathbf{x})} = -2 \sum_{i=1}^{n} g(\mathbf{p}_{m,i}, \mathbf{x}; \mathbf{\theta}) \left( u_{m,i} g(\mathbf{p}_{m,i}, \mathbf{x}; \mathbf{\theta}) - u(\mathbf{x}) g(\mathbf{p}_{m,i}, \mathbf{x}; \mathbf{\theta}) \right).$

These equations represent the misfit value and its gradient computed by the `MisfitReporterOffsetFunctionMaterial` material at each quadrature point in the simulation. The misfit value quantifies the discrepancy between the measured data and the simulation variable, while the misfit gradient provides the sensitivity of the misfit with respect to changes in the simulation variable.

## Example Input File Syntax

The following input file contains an example of how to use this material to define gaussian shaped measurement points in a heat source inversion problem where the misfit is used in the computation of the objective function and the misfit_gradient is used to compute the right hand side in the adjoint problem.

!listing test/tests/optimizationreporter/function_misfit/forward_and_adjoint.i block=Materials/beam

## Syntax parameters and inputs

!syntax parameters /Materials/MisfitReporterOffsetFunctionMaterial

!syntax inputs /Materials/MisfitReporterOffsetFunctionMaterial

!syntax children /Materials/MisfitReporterOffsetFunctionMaterial
