# ConvectiveFluxFunction

!syntax description /BCs/ConvectiveFluxFunction

## Description

The `ConvectiveFluxFunction` boundary condition is used to prescribe the following convective flux
$\dot{q}$ to a boundary of a thermal model:
\begin{equation}
   \dot{q} = h(T(x) - T_{\text{inf}})
\end{equation}
where $h$ is the heat transfer coefficient, $T(x)$ is the temperature at a location $x$, where $x$ is
the location of a given quadrature point on the surface where this boundary condition is applied, and
$T_{\text{inf}}$ is the far-field temperature of the fluid that the boundary is exposed to.

The far-field temperature is specified using the `T_infinity` parameter, which is a MOOSE Function. This can be provided as the name of a function, using a parsed function, or a constant value.  Likewise, the heat transfer coefficient is specified using the `coefficient` parameter, which is also a MOOSE Function, and the same options for how to prescribe `T_infinity` also apply for this parameter. By default, the heat transfer coefficient function is defined in terms of position and time (the standard usage of MOOSE Functions). If the optional `coefficient_function_type` parameter is set to `TEMPERATURE`, the coefficient is instead a function of the temperature. If a Function such as `PiecewiseLinear` were used to define this coefficient, the values on the x-axis of that function would be interpreted as temperature values. If a parsed function were used for this purpose, the `t` variable in a parsed equation would be used for the temperature, rather than for the time.

!syntax parameters /BCs/ConvectiveFluxFunction

!syntax inputs /BCs/ConvectiveFluxFunction

!syntax children /BCs/ConvectiveFluxFunction
