# CaloricallyImperfectGas

!syntax description /Modules/FluidProperties/CaloricallyImperfectGas

This class implements fluid properties for a gas that behaves like an
ideal gas except that the specific heat capacities are a function of temperature
(as opposed to constants as for the ideal gas).

The relationship between pressure, density, and temperature is identical to an ideal
gas:

\begin{equation}
  pv = R_s T,
\end{equation}  

where $p$ is pressure, $v$ is specific volume, $R_s$ is the specific
gas constant, and $T$ is temperature.

The internal energy $e$ is a user-provided function of temperature:

\begin{equation}
  e = e(T).
\end{equation}  

The function $e(T)$ is provided via parameter `e`. The time argument is interpreted
as temperature.

The enthalpy is computed by:

\begin{equation}
  h = e(T) + R_s T.
\end{equation}  

The specific heat capacities at constant volume and pressure $c_v$ and $c_p$
are computed by:

!equation
\begin{aligned}
c_v &= \frac{de}{dT} \\
c_p &= \frac{de}{dT} + R_s.
\end{aligned}

The specific heat capacities are computed from $e(T)$ using the `timeDerivative` method
of the `Function` class. The type of function that is used for $e(T)$ *must* implement
the `timeDerivative` method.

The inverse functions $T(e)$ and $T(h)$ are obtained as follows:

1. Ensure that within the acceptable temperature range (parameters `min_temperature` and `max_temperature`) $c_v > 0$.

2. Compute the minimum and maximum values of $e$ and $h$.

!equation
\begin{aligned}
e_{min} &= e(T_{min}) \\
e_{max} &= e(T_{max}) \\
h_{min} &= h(T_{min}) \\
h_{max} &= h(T_{max}) \\
\end{aligned}

3. Sample

\begin{equation}
e_j = e_{min} + j \frac{e_{max}-e_{min}}{N}, j=0,..,N.
\end{equation}

and for each $e_j$ solve

\begin{equation}
e_j - e(T_j)= 0,
\end{equation}

for $T_j$. Create tabulation ${e_j, T_j}$. Create a similar tabulation for enthalpy.

4. Evaluating $T(e)$ uses linear interpolation in ${e_j, T_j}$.

!syntax parameters /Modules/FluidProperties/CaloricallyImperfectGas

!syntax inputs /Modules/FluidProperties/CaloricallyImperfectGas

!syntax children /Modules/FluidProperties/CaloricallyImperfectGas

!bibtex bibliography
