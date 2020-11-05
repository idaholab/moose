# PorousFlowHysteresisOrder

This Material is used in simulations involving [hysteresis](porous_flow/hysteresis.md).

This Material computes the "order" for simulations involving hysteretic capillary-pressure and/or relative-permeability functions.  It also records the liquid saturation at each turning point.  The meaning of "order" is illustrated in [hysteretic_order_fig].
The order and the turning points may be recorded into AuxVariables using a [PorousFlowPropertyAux](PorousFlowPropertyAux.md).

Since the hysteretic capillary-pressure and relative-permeability functions are only defined up to third order, the order computed by `PorousFlowHysteresisOrder` never exceeds 3 (hard coded in `PorousFlowConstants.h`).

!media media/porous_flow/hysteretic_order.png caption=An illustration of hysteresis order.  The liquid-saturation turning points are marked as TP$_{n}$.  id=hysteretic_order_fig


!syntax parameters /Materials/PorousFlowHysteresisOrder

!syntax inputs /Materials/PorousFlowHysteresisOrder

!syntax children /Materials/PorousFlowHysteresisOrder
