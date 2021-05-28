# CNSFVHLLCStagnationInletBC

This is the base object for applying a stagnation boundary condition to the
mass, momentum, and energy equations. `stagnation_temperature` and
`stagnation_pressure` are required parameters of any objects derived from this
class. `CNSFVHLLCStagnationInletBC` computes several boundary quantities from
the supplied stagnation temperature and pressure. Boundary temperature is
computed with the assumptions laid out
[here](https://en.wikipedia.org/wiki/Stagnation_temperature) via the relation:

\begin{equation}
T_b = T_0 - \frac{V^2}{2c_p}
\end{equation}

where $T_b$ is the boundary temperature, $T_0$ is the stagnation temperature,
$V$ is the velocity and $c_p$ is the specific heat capacity. In
`CNSFVHLLCStagnationInletBC` $V$ is drawn from the cell centroid bordering the
boundary, e.g. it is implicit. With $T_b$ computed, the static pressure at the
boundary is computed using the isentropic relation:

\begin{equation}
p_b = p_0 \left(\frac{T_0}{T_b}\right)^{\frac{-\gamma}{\gamma - 1}}
\end{equation}

where $p_0$ is the stagnation pressure and $\gamma$ is the ratio of the specific
heats, e.g. $c_p/c_v$.

Armed with the boundary pressure and temperature, `CNSFVHLLCStagnationInletBC`
also computes the boundary values for the specific internal energy $e$, density
$\rho$, and specific total enthalpy $h_t$ for use the the flux expressions of
the derived [mass](CNSFVHLLCMassStagnationInletBC.md),
[momentum](CNSFVHLLCMomentumStagnationInletBC.md), and
[energy](CNSFVHLLCFluidEnergyStagnationInletBC.md) classes.
