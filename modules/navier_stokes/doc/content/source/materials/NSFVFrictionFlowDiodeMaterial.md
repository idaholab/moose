# NSFVFrictionFlowDiodeMaterial

!syntax description /Materials/NSFVFrictionFlowDiodeMaterial

This material is meant to be used to implement a simplistic volumetric flow diode. The
parameter [!param](/Materials/NSFVFrictionFlowDiodeMaterial/additional_linear_resistance)
and [!param](/Materials/NSFVFrictionFlowDiodeMaterial/additional_quadratic_resistance) should be chosen such that
the flow is sufficiently vanished in the direction opposite the diode.

## Example input file syntax

In this example the friction flow diode is added to a porous media simulation.
The `combined_linear/quadratic` friction coefficients contain both the diode
friction coefficients and the base porous media friction.

The friction coefficients are then combined using a [PiecewiseByBlockVectorFunctorMaterial.md] to have a uniform name over the whole domain for friction coefficients.

!listing test/tests/finite_volume/materials/flow_diode/friction.i block=Materials

!syntax parameters /Materials/NSFVFrictionFlowDiodeMaterial

!syntax inputs /Materials/NSFVFrictionFlowDiodeMaterial

!syntax children /Materials/NSFVFrictionFlowDiodeMaterial
