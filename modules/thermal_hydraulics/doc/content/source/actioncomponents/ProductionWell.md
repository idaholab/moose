# ProductionWell

This [ActionComponent.md] creates a production well, composed of the following components:

- [Outlet1Phase.md], located at the "surface point"
- [FlowChannel1Phase.md]
- [VolumeJunction1Phase.md]
- [VolumeJunctionCoupledFlux1Phase.md]
- [SolidWall1Phase.md], optionally located at the "bottom" of the well

The user provides the surface point via [!param](/ActionComponents/ProductionWell/surface_point),
the junction points with [!param](/ActionComponents/ProductionWell/junction_points), and
then an optional end point with [!param](/ActionComponents/ProductionWell/end_point)
(if omitted, there is no channel after the last junction).

If [!param](/ActionComponents/ProductionWell/multi_app) is provided, it is passed
to [VolumeJunctionCoupledFlux1Phase.md] (see its documentation for more information).

The flow channels use [Closures1PhaseSimple.md] with the friction factor provided by
[!param](/ActionComponents/ProductionWell/friction_factor).

The outlet pressure is specified by
[!param](/ActionComponents/ProductionWell/outlet_pressure).

!syntax parameters /ActionComponents/ProductionWell

!syntax inputs /ActionComponents/ProductionWell

!syntax children /ActionComponents/ProductionWell
