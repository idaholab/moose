# CaloricallyImperfectGas

!syntax description /FluidProperties/CaloricallyImperfectGas

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

The function $e(T)$ is provided via parameter [!param](/FluidProperties/CaloricallyImperfectGas/e). The time argument is interpreted
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

!alert warning
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
   e_j = e_{min} + j \frac{e_{max}-e_{min}}{N},\qquad j=0,..,N.
   \end{equation}

   and for each $e_j$ solve

   \begin{equation}
   e_j - e(T_j)= 0,
   \end{equation}

   for $T_j$. Create tabulation $\{e_j, T_j\}$. Create a similar tabulation for enthalpy.

4. Evaluating $T(e)$ uses linear interpolation in $\{e_j, T_j\}$.

## Evaluating Entropy

From the first and second law of thermodynamics is follows that:

!equation
\begin{aligned}
dq &= de + pdv \\
ds &= dq/T,
\end{aligned}

where $dq$ is an infinitesimal amount of heat. Solving for $ds$ and using the ideal gas law leads to:

\begin{equation}
 ds = \frac{de}{T} + R_s \frac{dv}{v}.
\end{equation}

Integrating this equation leads to an expression for the entropy:

\begin{equation}
 s(e, v) - s(e_0, v_0) = \int\limits_{e_0}^e \frac{de'}{T'} + R_s \log \frac{v}{v_0},
\end{equation}

where the prime indicates that these variables are dummy variables of integration.
We are free to select a zero-point for entropy and we select $s(e_0, v_0) = 0$.
We also select $v_0 = 1$ and $e_0 = e(T_{min})$. Then we define:

\begin{equation}
 Z(T) = \int\limits_{e_0}^{e(T)} \frac{de'}{T'} = \int\limits_{T_{min}}^{T} \frac{c_v(T')}{T'} dT'.
\end{equation}

Entropy is computed from the expression:

\begin{equation}
 s(e, v) = Z(T(e)) + R_s \log v,
\end{equation}

where $Z(T)$ is computed using a trapezoidal rule and tabulated between $T_{min}$ and $T_{max}$.
It is linearly interpolated.

The derivatives of entropy with respect to $T$, $e$, $h$, $v$ are given by:

!equation
\begin{aligned}
\left(\frac{\partial s}{\partial e}\right)_v &= \frac{1}{T(e)} \\
\left(\frac{\partial s}{\partial v}\right)_e &= \frac{R_s}{v} \\
\left(\frac{\partial s}{\partial p}\right)_T &= -\frac{R_s}{p} \\
\left(\frac{\partial s}{\partial T}\right)_p &= \frac{c_p(T)}{T} \\
\left(\frac{\partial s}{\partial p}\right)_h &= -\frac{R_s}{p} \\
\left(\frac{\partial s}{\partial h}\right)_p &= \frac{1}{T}.
\end{aligned}

These derivatives are implemented in the 5 argument versions of the `s_from_x_y`
functions.

!syntax parameters /FluidProperties/CaloricallyImperfectGas

!syntax inputs /FluidProperties/CaloricallyImperfectGas

!syntax children /FluidProperties/CaloricallyImperfectGas

!bibtex bibliography
