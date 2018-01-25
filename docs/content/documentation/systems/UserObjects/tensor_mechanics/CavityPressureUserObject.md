# Cavity Pressure UserObject
!syntax description /UserObjects/CavityPressureUserObject

## Description
The `CavityPressureUserObject` is used to compute: both the number of initial moles from a gas contained in an internal volume and the pressure exterted by a gas on the cavity boundary.
This postprocessor is suitable only for ideal gases, which obey the ideal gas law:
\begin{equation}
  \label{eq:ideal_gas_law}
  P=\frac{nRT}{V}
\end{equation}
where $P$ is the internal pressure, $n$ is the moles of gas, $R$ is the ideal gas constant, $T$ is the temperature, and $V$ is the volume of the cavity.
To compute the initial number of moles, \eqref{eq:ideal_gas_law} is rearranged to solve for moles from an initial pressure.

The moles of gas, the temperature, and the cavity volume in \eqref{eq:ideal_gas_law} are free to change with time.  The moles of gas $n$ at any time is the original amount of gas (computed based on original pressure, temperature, and volume) plus the amount in the cavity due to any gas injected during the simulation.

!syntax parameters /UserObjects/CavityPressureUserObject

!syntax inputs /UserObjects/CavityPressureUserObject

!syntax children /UserObjects/CavityPressureUserObject
