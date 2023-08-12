# HeatStructure2DRadiationCouplerRZBC

!syntax description /BCs/HeatStructure2DRadiationCouplerRZBC

The heat flux contribution to the residual $R_i$ for the weak form of the energy equation is computed as:

\begin{equation}
R_i = (\psi_i, _sigma * \dfrac{T^4 - T_{coupled}^4}{R} C \quad \forall \psi_i,
\end{equation}

where $\psi_i$ are the test functions, $\sigma$ is the Stefan-Boltzmann constant, $T$ is one heat structure temperature
variable, $T_{coupled}$ the other heat structure temperature variable, $R$ the radiation resistance, and $C$ the
circumference of the heat structure boundary considered.

The radiation resistance is computed as:

!equation
R = \dfrac{1.0 - \epsilon}{\epsilon} + \dfrac{1.0}{V} +
    \dfrac{1.0 - \epsilon_{coupled}}{\epsilon_{coupled}}  \dfrac{A}{A_{coupled}}

where $\epsilon$ is the emissivity of one heat structure boundary, $\epsilon_{coupled}$ the surface emissivity
of the other heat structure's boundary, $V$ the view factor between the two boundaries, $A$ the surface area,
and $A_{coupled}$ the surface area of the coupled heat structure's boundary.

!alert warning
This boundary condition is meant to be used in XY coordinates that are interpreted as general cylindrical coordinates.
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general RZ-coordinate objects.
Stay tuned!

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by the
[HeatStructure2DRadiationCouplerRZ.md] to couple boundaries of two 2D cylindrical heat structures via radiation.
The boundary condition is added once for each cylindrical heat structure.

!syntax parameters /BCs/HeatStructure2DRadiationCouplerRZBC

!syntax inputs /BCs/HeatStructure2DRadiationCouplerRZBC

!syntax children /BCs/HeatStructure2DRadiationCouplerRZBC
