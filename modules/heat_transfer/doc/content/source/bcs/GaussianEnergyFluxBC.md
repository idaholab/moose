# GaussianEnergyFluxBC

!syntax description /BCs/GaussianEnergyFluxBC

This boundary condition computes an influx of energy from a beam (e.g. laser) with a
Gaussian spatial profile. The flux is given by

\begin{equation}
-\frac{2P_0}{\pi R^2}\exp{\frac{-2r_p^2}{R^2}}
\end{equation}

where $P_0$ is the total power of the beam, $R$ is the radius at which the
intensity falls to $1/\exp{2}$ of its axial value, and $r_p$ is the normed
distance from the point at which we're evaluating the flux to the centerpoint of
the beam. This functional form of the beam flux is taken from
[Wikipedia](https://en.wikipedia.org/wiki/Gaussian_beam). The negative sign on
the flux indicates that the flux is incoming. This class assumes that the beam
impinges perpendicular to the surface.

!syntax parameters /BCs/GaussianEnergyFluxBC

!syntax inputs /BCs/GaussianEnergyFluxBC

!syntax children /BCs/GaussianEnergyFluxBC
