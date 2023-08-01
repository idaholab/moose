# OneDEnergyWallHeating

!syntax description /Kernels/OneDEnergyWallHeating

The wall heating is expressed as a convection term:

!equation
H_w P_{hf} (T_{fluid} - T_{wall})

where $H_w$ is the convective heat transfer coefficient, $P_{hf}$ is the heated perimeter,
$T_{fluid}$ is the fluid temperature and $T_{wall}$ is the wall temperature.

The dependence of the fluid temperature on the conserved variables $\rhoA$, $\rho uA$ and $\rho EA$
is modeled by retrieving the derivatives of the temperature as material properties, and using them to
contribute to the Jacobian.

!alert note
In THM, most kernels are added automatically by components. This kernel is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADOneDEnergyWallHeating.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /Kernels/OneDEnergyWallHeating

!syntax inputs /Kernels/OneDEnergyWallHeating

!syntax children /Kernels/OneDEnergyWallHeating
