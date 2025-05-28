# LinearFVConvectiveHeatTransferBC

## Description

This boundary condition describes correlation-based convective heat transfer between solid
and fluid domains. The heat flux from the fluid's perspective can be described as:

\begin{equation}
q^{''} = h (T_{solid}-T_{fluid}),
\end{equation}

where $h$ is the user supplied heat transfer coefficient. The variables
can be supplied though [!param](/LinearFVBCs/LinearFVConvectiveHeatTransferBC/T_fluid)
and [!param](/LinearFVBCs/LinearFVConvectiveHeatTransferBC/T_solid) parameters.
This object will automatically detect which variable matches the
object's and acts accordingly. To account for the heat flux on both sides of the interface,
the boundary condition should be used by both variables as
shown in:

!listing modules/navier_stokes/test/tests/finite_volume/ins/cht/flow-around-square-linear.i block=LinearFVBCs

!syntax parameters /LinearFVBCs/LinearFVConvectiveHeatTransferBC

!syntax inputs /LinearFVBCs/LinearFVConvectiveHeatTransferBC

!syntax children /LinearFVBCs/LinearFVConvectiveHeatTransferBC
