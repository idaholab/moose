# CoupledForceRZ

!syntax description /Kernels/CoupledForceRZ

`CoupledForceRZ` implements a source term
within a cylindrical domain $\Omega$ (represented in 2D XY) proportional to a coupled variable:

\begin{equation}
\underbrace{-\sigma v}_{\textrm{CoupledForce}} + \sum_{i=1}^n \beta_i = 0 \in \Omega,
\end{equation}

where $\sigma$ is a known scalar coefficient, $v$ is a coupled unknown value, and the second term on
the left hand side corresponds to the strong forms of other kernels.

!alert warning
This kernel is meant to be used in XY coordinates that are interpreted as general cylindrical coordinates.
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general RZ-coordinate objects.
Stay tuned!

!alert note
In THM, most kernels are added automatically by components. This kernel is created by the
[HeatSourceFromPowerDensity.md] heat structure in the context of cylindrical geometries.

!syntax parameters /Kernels/CoupledForceRZ

!syntax inputs /Kernels/CoupledForceRZ

!syntax children /Kernels/CoupledForceRZ
