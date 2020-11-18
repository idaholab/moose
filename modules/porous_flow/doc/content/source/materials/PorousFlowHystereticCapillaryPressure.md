# PorousFlowHystereticCapillaryPressure

This is a base class that enables computation of capillary pressure, saturations and porepressures in models with [hysteresis](hysteresis.md).  It should not usually appear in MOOSE input files.  Its derived classes, which should be employed in MOOSE input files, are:

- [PorousFlow1PhaseHysP](PorousFlow1PhaseHysP.md) which computes the saturation given the porepressure in partially-saturated 1-phase systems
- [PorousFlowHystereticInfo](PorousFlowHystereticInfo.md) which computes the capillary pressure, and other quantities, given the saturation.


!syntax parameters /Materials/PorousFlowHystereticCapillaryPressure

!syntax inputs /Materials/PorousFlowHystereticCapillaryPressure

!syntax children /Materials/PorousFlowHystereticCapillaryPressure
