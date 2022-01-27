# Shaft

This component implements a shaft used to connect components such as a turbine, pump, and motor. The shaft provides a rigid connection between connected components, thus there is only one rotational speed shared by all connections. The shaft receives torque and inertia coupled variables from the components connected to it. The angular speed of the shaft and connected components is dictated by the torque-inertia balance,

\begin{equation}
  \sum_{k=1}^{n} I_{k} \frac{d \omega}{dt} = \sum_{k=1}^{n} \tau_{k} \,,
\end{equation}
where

- $\omega$ is the angular speed of the shaft and all connected components,
- $I_{k}$ is the moment of inertia for the $k^{th}$ connection,
- $\tau_{k}$ is the net torque for the $k^{th}$ connection, and
- $n$ is the number of components connected to the shaft.

!syntax parameters /Components/Shaft

!syntax inputs /Components/Shaft

!syntax children /Components/Shaft
