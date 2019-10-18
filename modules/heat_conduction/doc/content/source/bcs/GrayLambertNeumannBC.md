# GrayLambertNeumannBC

!syntax description /BCs/GrayLambertNeumannBC

This boundary condition computes the heat flux density resulting from the
radiative heat transfer between surfaces adjacent to this boundary. These
surfaces must be diffuse, gray radiators (Lambert radiators). The heat flux
is computed by the net radiation method described in [!cite](modest2013radiative).
More information is available [here](userobjects/ConstantViewFactorSurfaceRadiation.md).

There are two modes available for operating this boundary condition. The mode can be switched
by the `reconstruct_emission` parameter. If set to `false`, the object queries the
net radiation object (`GrayLambertSurfaceRadiationBase` or derived class) object for the
net heat flux density on the surface. This heat flux density is applied as a constant for
each participating sideset. If the sidesets are large and the flat heat flux densities on two adjacent sidesets sufficiently different, then the temperature will be non-smooth.

A smoother temperature distribution on the surface is usually obtained by noting that a large
fraction of the spatial distribution of the heat flux stems from the temperature distribution
and hence from the emission. The approximation made is that the emission is allowed to spatially
vary, while the irradiation from other sidesets to the given sideset is assumed to be
spatially flat. The heat flux at location $\vec{x}$ on sideset $i$ is computed by:

\begin{equation}
   \dot{q}_i(\vec{x}) = \epsilon_i \left( \sigma T(\vec{x})^4 - H_i\right),
\end{equation}

where $\sigma$ is the Stefan-Boltzmann constat, $\epsilon_i$ the emissivity of sideset $i$, and
$H_i$ the average irradiation into sideset $i$.

!listing modules/heat_conduction/test/tests/gray_lambert_radiator/coupled_heat_conduction.i
block=BCs

!syntax parameters /BCs/GrayLambertNeumannBC

!syntax inputs /BCs/GrayLambertNeumannBC

!syntax children /BCs/GrayLambertNeumannBC
