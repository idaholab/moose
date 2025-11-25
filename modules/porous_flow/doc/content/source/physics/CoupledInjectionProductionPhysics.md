# CoupledInjectionProduction

This [Physics](syntax/Physics/index.md) object adds dirac kernels for mass and energy
sources at injection and production points within a porous matrix, where the sources
are provided by post-processor values transferred from another application.

This object creates the following post-processors:

- `p_inj1`, ..., `p_inj<n_inj>`: Pressure at each injection point, via [PointValue.md].
- `T_inj1`, ..., `T_inj<n_inj>`: Temperature at each injection point, via [PointValue.md].
- `p_pro1`, ..., `p_pro<n_pro>`: Pressure at each production point, via [PointValue.md].
- `T_pro1`, ..., `T_pro<n_pro>`: Temperature at each production point, via [PointValue.md].
- `mass_rate_inj1`, ..., `mass_rate_inj<n_inj>`: Mass source rate at each injection point, to be transferred from the other application using [Receiver.md].
- `energy_rate_inj1`, ..., `energy_rate_inj<n_inj>`: Energy source rate at each injection point, to be transferred from the other application using [Receiver.md].
- `mass_rate_pro1`, ..., `mass_rate_pro<n_pro>`: Mass source rate at each production point, to be transferred from the other application using [Receiver.md].
- `energy_rate_pro1`, ..., `energy_rate_pro<n_pro>`: Energy source rate at each production point, to be transferred from the other application using [Receiver.md].

`n_inj` is the size of the [!param](/Physics/CoupledInjectionProduction/CoupledInjectionProductionPhysics/injection_points) parameter, and
`n_pro` is the size of the [!param](/Physics/CoupledInjectionProduction/CoupledInjectionProductionPhysics/production_points) parameter.

If the parameter [!param](/Physics/CoupledInjectionProduction/CoupledInjectionProductionPhysics/multi_app)
is provided, then the porous flow app is the main application, and [MultiAppPostprocessorTransfer.md]
objects are created by this object to transfer the post-processors listed above to the sub application;
if the parameter is not provided, then the other application is responsible for creating the transfers.

[PorousFlowPointSourceFromPostprocessor.md] dirac kernels are added for each injection and production point,
using the `mass_rate_*` and `energy_rate_*` post-processors listed above.

!syntax parameters /Physics/CoupledInjectionProduction/CoupledInjectionProductionPhysics

!syntax inputs /Physics/CoupledInjectionProduction/CoupledInjectionProductionPhysics

!syntax children /Physics/CoupledInjectionProduction/CoupledInjectionProductionPhysics
