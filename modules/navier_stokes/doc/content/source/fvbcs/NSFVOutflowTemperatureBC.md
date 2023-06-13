# NSFVOutflowTemperatureBC

This boundary condition allows the user to specify the backflow temperature in problems that have flow inversion at a boundary.
In case the flow is going outside of the domain, i.e., $\vec{u} \cdot \vec{n} > 0$, the enthalpy flux is defined as follows:

\begin{equation}
h_{out} = \rho c_p T,
\end{equation}

where $T$ is the extrapolated temperature from the domain to the boundary.


In case the flow is coming into the domain, i.e., $\vec{u} \cdot \vec{n} < 0$, the enthalpy flux is defined as follows:

\begin{equation}
h_{in} = \rho c_p T_{backflow},
\end{equation}
where $T_{backflow}$ is a user-specified backflow temperature.

!syntax parameters /FVBCs/NSFVOutflowTemperatureBC

!syntax inputs /FVBCs/NSFVOutflowTemperatureBC

!syntax children /FVBCs/NSFVOutflowTemperatureBC
