# Out-Of-Plane Pressure

!syntax description /Kernels/OutOfPlanePressure

## Description

The `OutOfPlanePressure` kernel applies an out-of-plane pressure value in the
out-of-plane direction of [Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)
or 2D plane stress problems.
Either a function or a postprocessor can be used to specify the value of the
out-of-plane pressure at each quadrature point.
\begin{equation}
  \label{eqn:out_of_plane_pressure}
  P = f \cdot \hat{n}
\end{equation}
where $P$ is the computed pressure, $f$ is the function or postprocessor value of
the pressure to be applied, and $\hat{n}$ is the unit normal vector to the out-of-plane
surface.
Following the convention of the [Pressure](bcs/Pressure.md) boundary condition,
the unit normal vector is considered to be positive when pointing inward towards
the surface.

!syntax parameters /Kernels/OutOfPlanePressure

!syntax inputs /Kernels/OutOfPlanePressure

!syntax children /Kernels/OutOfPlanePressure
