# Density Update for Multimaterial Topology Optimization

!syntax description /UserObjects/DensityUpdateTwoConstraints

## Description

The `DensityUpdateTwoConstraints` class is an `ElementUserObject`
that calculates and updates the design densities based on the
filtered cost and compliance sensitivities. The density update is performed
using the multi-material Solid Isotropic Material with Penalization
(SIMP) method (see [!cite](zuo2017multi)). The class requires
the design density variable, the filtered
design density variable, the density sensitivity variable, the penalty power,
the volume fraction, the cost fraction, the cost variable, and the cost
sensitivity variable as inputs.

## Example Input File

An example of how to use the `DensityUpdateTwoConstraints` class in an input file:

listing test/tests/materials/compliance_sensitivity/2d_mmb_2material_cost_initial.i block=UserObjects/update


!syntax parameters /UserObjects/DensityUpdateTwoConstraints

!syntax inputs /UserObjects/DensityUpdateTwoConstraints

!syntax children /UserObjects/DensityUpdateTwoConstraints

!tag name=DensityUpdateTwoConstraints pairs=module:optimization system:userobjects
