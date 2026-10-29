# ThermalHydraulics Physics syntax

The `[ThermalHydraulics]` block is a sub-block of the `[Physics]` block. It holds the
[Physics](syntax/Physics/index.md) objects that define the equations of a thermal hydraulics
[ActionComponent](ActionComponent.md), such as [FlowChannel1PhasePhysics.md] for a
[FlowChannel1PhaseAC.md]. These Physics are created by the component and do not need to be
declared in an input file.

!syntax list /Physics/ThermalHydraulics objects=True actions=False subsystems=False

!syntax list /Physics/ThermalHydraulics objects=False actions=False subsystems=True

!syntax list /Physics/ThermalHydraulics objects=False actions=True subsystems=False
