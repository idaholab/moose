# TemperaturePressureFunctionFluidProperties

!syntax description /FluidProperties/TemperaturePressureFunctionFluidProperties

The temperature and pressure will be passed as respectively the x and y spatial arguments of the
functions. The following relations hold true:

\begin{equation}
\begin{aligned}
  x = T \\
  y = P \\
  \rho = \rho^{u}(t=0, (x=T, y=P, z=0)) \\
  \mu = \mu^{u}(t=0, (x=T, y=P, z=0)) \\
  k = k^{u}(t=0, (x=T, y=P, z=0)) \\
\end{aligned}
\end{equation}

Both the time (`t`) and Z-axis dimension are not used here. A fluid property made to depend on time will
not be properly updated by this `FluidProperties` object.
There are two options for specific heat. Either the user sets a constant specific isochoric heat capacity

\begin{equation}
\begin{aligned}
  c_v = c_v^{u} \\
  e = e_{ref} + c_v * (T - T_{ref})
\end{aligned}
\end{equation}

Or, the user uses a function of temperature and pressure (same arguments as for density)

\begin{equation}
\begin{aligned}
  x = T \\
  y = P \\
  cp = cp^{u}(t=0, (x=T, y=P, z=0))
  cv = cp - \dfrac{\alpha^2 T}{\rho \beta_T}
\end{aligned}
\end{equation}

with $T$ the temperature, $P$ the pressure, $\rho$ the density, $\mu$ the dynamic viscosity, $k$ the thermal conductivity, $c_v$ the specific isochoric heat capacity, $e$ the specific internal energy, $T_{ref}$ a reference temperature at which the specific internal energy is equal to a reference energy $e_{ref}$, $\alpha$ the coefficient of thermal expansion, $\beta_T$ the isothermal
compressibility, and the $^u$ exponent indicating a user-passed parameter.

The derivatives of the fluid properties are obtained using the `Function`(s) gradient components
and the appropriate derivative chaining for derived properties.

!alert warning
The range of validity of the property is based off of the validity of the functions
that are input, and is not checked by this `FluidProperties` object.

!alert note
Support for the conservative (specific volume, internal energy) variable set is only
partial. Notable missing implementations are routines for entropy, the speed of sound, and some
conversions between specific enthalpy and specific energy.

!alert note
When using a function for the isobaric specific heat capacity, a numerical integration is performed to compute
$e(p,T)$ as $e_{ref} + \int_{T_{ref}}^T c_v(p,T) dT$. Note that this neglects the $dV$ term. This is exact
for incompressible fluids and ideal gases.

## Example Input File Syntax

In this example, temperature and pressure dependent density, dynamic viscosity and thermal conductivity of a fluid are being set using
three `ParsedFunctions`. The functions are specified with the `x` and `y` coordinates, and are evaluated with respectively the temperature and
pressure variables.

!listing modules/fluid_properties/test/tests/temperature_pressure_function/example.i block=Functions FluidProperties

!syntax parameters /FluidProperties/TemperaturePressureFunctionFluidProperties

!syntax inputs /FluidProperties/TemperaturePressureFunctionFluidProperties

!syntax children /FluidProperties/TemperaturePressureFunctionFluidProperties
