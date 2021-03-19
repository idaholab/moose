# PorousFlow1PhaseHysP

This Material computes saturation given porepressure in single-phase, partially-saturated, hysteretic situations, assuming a van Genuchten relationship.  Detailed documentation about hysteresis and the van Genuchten relationship can be found on the [hysteresis page](hysteresis.md).  It is the hysteretic cousin of [PorousFlow1PhaseP](PorousFlow1PhaseP.md)

This Material requires a [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material, which will compute the hysteresis order.  `PorousFlow1PhaseHysP` then uses the hysteresis order along with the single-phase porepressure to compute the saturation.

A simple example usage is:

!listing modules/porous_flow/test/tests/hysteresis/1phase.i

Note the following features:

- The [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material appears alongside the `PorousFlow1PhaseHysP` Material:

!listing modules/porous_flow/test/tests/hysteresis/1phase.i start=[hys_order_material] end=[Postprocessors]

- Saturation and hysteresis order may be recorded into `AuxVariables` using a [PorousFlowPropertyAux](PorousFlowPropertyAux.md) 

!listing modules/porous_flow/test/tests/hysteresis/1phase.i block=AuxKernels

!alert warning
To improve numerical convergence, it is recommended that you use a `low_extension` and a `high_extension` along with values of `Pc_max` and `S_lr` that are appropriate for your situation.  See the [hysteresis page](hysteresis.md) for more details.



!syntax parameters /Materials/PorousFlow1PhaseHysP

!syntax inputs /Materials/PorousFlow1PhaseHysP

!syntax children /Materials/PorousFlow1PhaseHysP
