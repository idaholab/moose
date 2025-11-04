# InjectionWell

This [ActionComponent.md] creates an injection well, composed of the following components:

- [InletMassFlowRateTemperature1Phase.md], located at the "surface point"
- [FlowChannel1Phase.md]
- [VolumeJunction1Phase.md]
- [VolumeJunctionCoupledFlux1Phase.md]
- [SolidWall1Phase.md], optionally located at the "bottom" of the well

The user provides the surface point via [!param](/ActionComponents/InjectionWell/surface_point),
the junction points with [!param](/ActionComponents/InjectionWell/junction_points), and
then an optional end point with [!param](/ActionComponents/InjectionWell/end_point)
(if omitted, there is no channel after the last junction).

If [!param](/ActionComponents/InjectionWell/multi_app) is provided, it is passed
to `VolumeJunctionCoupledFlux1Phase` (see its documentation for more information).

The flow channels use [Closures1PhaseSimple.md] with the friction factor provided by
[!param](/ActionComponents/InjectionWell/friction_factor).

The inlet mass flow rate and temperature are specified by
[!param](/ActionComponents/InjectionWell/inlet_mass_flow_rate) and
[!param](/ActionComponents/InjectionWell/inlet_temperature), respectively.

!syntax parameters /ActionComponents/InjectionWell

!syntax inputs /ActionComponents/InjectionWell

!syntax children /ActionComponents/InjectionWell
