# NSFVFrictionFlowDiodeMaterial

!syntax description /Materials/NSFVFrictionFlowDiodeMaterial

This material is meant to be used to implement a simplistic volumetric flow diode. The
parameter [!param](/Materials/NSFVFrictionFlowDiodeMaterial/additional_linear_resistance)
and [!param](/Materials/NSFVFrictionFlowDiodeMaterial/additional_quadratic_resistance) should be chosen such that
the flow is sufficiently vanished in the direction opposite the diode.

!alert warning
The operation of the diode is controlled with the [!param](/Materials/NSFVFrictionFlowDiodeMaterial/turn_on_diode)
parameter. If this parameter is false, the 'diode' does *NOT* create any flow restriction.

## Example input file syntax

### Simple always-on friction term

In this example the friction flow diode is added to a porous media simulation.
The `combined_linear/quadratic` friction coefficients contain both the diode
friction coefficients and the base porous media friction.

The friction coefficients are then combined using a [PiecewiseByBlockVectorFunctorMaterial.md] to have a uniform name over the whole domain for friction coefficients.

!listing test/tests/finite_volume/materials/flow_diode/friction.i block=Materials

### Dynamic operation of a diode

In this example, we show three [Control](syntax/Controls/index.md) strategies for the diode.
The idea of these controls is to detect a condition in which the flow should be blocked, because it's going
in the way opposite the direction of the diode, or because it meets a criterion that is outlined in the postprocessors
and functions involved to describe it.

The first strategy is simply to block the flow at a given time.

!listing test/tests/finite_volume/materials/flow_diode/transient_operation.i block=Controls/time_based Functions/time_function

The second strategy is to look at the pressure drop across the diode, and block (add friction to) the flow if it exceeds a certain value.
If it exceeds a certain value, then in all likelihood means that the flow is flowing through the diode in the direction of decreasing pressure.

!listing test/tests/finite_volume/materials/flow_diode/transient_operation.i block=Controls/pdrop_based Postprocessors/pdrop_diode Functions/pdrop_positive

The final strategy is to compute the mass flow rate through the diode, and block (add friction to) the flow if it exceeds a certain value.

!listing test/tests/finite_volume/materials/flow_diode/transient_operation.i block=Controls/flow_based Postprocessors/flow_diode Functions/velocity_big_enough

!alert note
All these strategies are workarounds for the fact that looking at the local velocity (in multi-dimensional space)
to apply a friction term based on this local velocity, rather than an average quantity, seems to be numerically unstable.

!syntax parameters /Materials/NSFVFrictionFlowDiodeMaterial

!syntax inputs /Materials/NSFVFrictionFlowDiodeMaterial

!syntax children /Materials/NSFVFrictionFlowDiodeMaterial
