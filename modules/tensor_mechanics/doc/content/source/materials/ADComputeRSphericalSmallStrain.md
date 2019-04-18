# Compute R-Spherical Small Strain

!syntax description /Materials/ADComputeRSphericalSmallStrain

## Description

The material `ADComputeRSphericalSmallStrain` calculates the small total strain
for 1D R-Spherical systems.

The 1D RSpherical materials and kernel are designed to model sphere geometries
with 1D models. Symmetry in the polar ($\theta$) and azimuthal ($\phi$)
directions is assumed, and the model is considered to revolve in both of these
directions.  In the 1D R-Spherical code, the material properties, variables
(e.g. temperature), and loading conditions are all assumed to be spherically
symmetric: these attributes only depend on the axial position.

!alert note title=Use `RSPHERICAL` Coordinate Type
The coordinate type in the `[Problem]` block of the input file must be set to
`coord_type = RSPHERICAL`.

As in the plane strain and axisymmetric cases, the stress and strain tensors are
modified in the spherical problem; only the diagonal components are non-zero in
this 1D problem.

\begin{equation}
\begin{bmatrix}
\epsilon_{rr} & 0 & 0 \\
0 & \epsilon_{\theta \theta} & 0 \\
0 & 0 & \epsilon_{\phi \phi}
\end{bmatrix}
\end{equation}

where the value of the normal strain components in the polar and azimuth
directions $\epsilon_{\theta \theta}$ and $\epsilon_{\phi \phi}$ depends on the
displacement and position in the radial direction

\begin{equation}
  \label{eq:polar_azimuthal_rspherical_strains}
  \epsilon_{\theta \theta} = \epsilon_{\phi \phi} = \frac{u_r}{X_r}.
\end{equation}

Although axisymmetric problems solve for 3D stress and strain fields, the
problem is mathematically 1D. In the cylindrical coordinate axisymmetric system,
the values of stress and strain in the $\theta$ and $\phi$ directions do not
depend on the $\theta$ or $\phi$ coordinates.

The RSpherical specific `ADComputeRSphericalSmallStrain` class calculates the
radial strain as normally done for a small total strain material:

\begin{equation}
  \epsilon_{rr} = \nabla u_r
\end{equation}

while the calculation of the total strain components $\epsilon_{\theta \theta}$
and $\epsilon_{\phi \phi}$ are found with
[eq:polar_azimuthal_rspherical_strains].

!syntax parameters /Materials/ADComputeRSphericalSmallStrain

!syntax inputs /Materials/ADComputeRSphericalSmallStrain

!syntax children /Materials/ADComputeRSphericalSmallStrain
