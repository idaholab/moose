# GaussianWeldEnergyFluxBC

!syntax description /BCs/GaussianWeldEnergyFluxBC

This boundary condition computes an influx of energy from a laser spot with a
Gaussian spatial profile. The flux is given by

\begin{equation}
-2r_{eff}F_0\exp{-r_{eff}r_p^2/R^2}
\end{equation}

where $r_eff$ is an effective radius used to specify the radial distribution of
beam energy, $F_0$ is the average heat flux of the laser, and $r_p$ is the
normed distance from the point at which we're evaluating the flux to the
centerpoint of the laser. This functional form of the laser flux is taken from
[!cite](noble2007use). The negative sign on the flux indicates that the flux is
incoming.

!syntax parameters /BCs/GaussianWeldEnergyFluxBC

!syntax inputs /BCs/GaussianWeldEnergyFluxBC

!syntax children /BCs/GaussianWeldEnergyFluxBC
