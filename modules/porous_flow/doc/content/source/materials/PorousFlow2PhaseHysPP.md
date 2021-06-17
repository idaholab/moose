# PorousFlow2PhaseHysPP

This Material computes the liquid and gas saturations given their porepressures for 2-phase hysteretic situations, assuming a van Genuchten relationship.  Detailed documentation about hysteresis and the van Genuchten relationship can be found on the [hysteresis page](hysteresis.md).  `PorousFlow2PhaseHysPP` is the hysteretic cousin of [PorousFlow2PhasePP](PorousFlow2PhasePP.md)

This Material requires a [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material, which will compute the hysteresis order.  `PorousFlow2PhaseHysPP` then uses the hysteresis order along with the two porepressures to compute the saturation.

A simple example usage is:

!listing modules/porous_flow/test/tests/hysteresis/2phasePP.i

Note the following features:

- The [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material appears alongside the `PorousFlow2PhaseHysPP` Material:

!listing modules/porous_flow/test/tests/hysteresis/2phasePP.i start=[hys_order_material] end=[Postprocessors]

- Saturations and hysteresis order may be recorded into `AuxVariables` using a [PorousFlowPropertyAux](PorousFlowPropertyAux.md) 

!listing modules/porous_flow/test/tests/hysteresis/2phasePP.i block=AuxKernels

!alert warning
To improve numerical convergence, it is recommended that you use a `low_extension` and a `high_extension` along with values of `Pc_max` and `S_lr` that are appropriate for your situation.  See the [hysteresis page](hysteresis.md) for more details.


!syntax parameters /Materials/PorousFlow2PhaseHysPP

!syntax inputs /Materials/PorousFlow2PhaseHysPP

!syntax children /Materials/PorousFlow2PhaseHysPP
