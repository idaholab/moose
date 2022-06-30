# Dipole Antenna Benchmark

This document summarizes and describes the half-wave dipole antenna benchmark.
In this scenario, a vertically-oriented antenna structure is excited by a 1 GHz
signal in an infinite vacuum. Then, the subsequent transmitted radiation is
absorbed by a first-order absorbing boundary condition (specifically, the
[VectorEMRobinBC.md] object), positioned five wavelengths away from the antenna
center-point.

## Model Geometry and Antenna Specifications

!style halign=left
A half-wave dipole antenna has a geometry shown in [dipole-geometry-figure].

!media dipole_geometry.png
  style=width:50%;margin:auto;
  id=dipole-geometry-figure
  caption=Half-wave dipole antenna with oscillating voltage and current standing wave.

The oscillating voltage between the two antenna conductors produces a current
standing wave distribution along the length of the dipole structure. The antenna
resonates (and transmits) when the applied voltage wave has a wavelength equal to
two times the dipole length. Thus, the resonant frequency of a half-wave dipole
is given by

!equation
f_r = \frac{c}{\lambda} = \frac{c}{2L}

where $f_r$ is the resonant frequency, $c$ is the speed of light, and
$L$ is the length of the half-wave dipole antenna. In this benchmark, a 1 GHz
resonant antenna will be modeled, and the parameters for the geometry are shown
in [dipole-geometry-parameters]. Note that the feed gap was chosen
so that it would be *much* smaller then the overall length of the antenna,
as an attempt to make the antenna geometry more ideal. The domain radius was
chosen to be suitably far from the antenna surface to optimize transmission
through the absorbing boundary condition. As mentioned in the [VectorEMRobinBC.md]
documentation, the first-order condition used was designed for flat surfaces, not
curved ones. Setting the boundary far enough from the origin increases the accuracy
of this assumption for this scenario.

!table id=dipole-geometry-parameters caption=Antenna geometry parameters for the dipole antenna verification benchmark.
| Parameter (unit) | Value |
| - | - |
| Resonant frequency (GHz) | 1 |
| Wavelength, $\lambda$ (m) | 0.3 |
| Antenna length, $L$ (m) | 0.15 |
| Antenna feed gap ($L / 20$) (m) | 0.0075 |
| Domain radius ($5 \lambda$) (m) | 1.5 |

## Governing Equations and Boundary Conditions

!style halign=left
In this simulation, both the real and imaginary components of the electric field
wave will be simulated separately as vector finite element variables (with the
magnitude displayed in [dipole-results-figure]), but the frequency-domain electric
field wave equation in general is given by:

!equation
\nabla \times \nabla \times \vec{E} - \mu_0 \varepsilon \omega^2 \vec{E} = 0

where

- $\vec{E}$ is the complex electric field vector in V/m,
- $\mu_0$ is the vacuum magnetic permeability of the medium in H/m,
- $\varepsilon$ is the electric permittivity of the medium in F/m, and
- $\omega$ is the operating frequency in rad/s.

In the EM module, these terms are represented by the [CurlCurlField.md] and
[VectorFunctionReaction.md] objects, respectively. At the edge of the simulation domain,
this model uses the [VectorEMRobinBC.md] object. This is given by

!equation
\hat{\mathbf{n}} \times \left( \frac{1}{\mu_0} \nabla \times \vec{E} \right) + j\beta(\mathbf{r}) \hat{\mathbf{n}} \times \left( \hat{\mathbf{n}} \times \vec{E} \right) = \hat{\mathbf{n}} \times (\nabla \times \vec{E}_{inc}) + j\beta(\mathbf{r}) \hat{\mathbf{n}} \times \left( \hat{\mathbf{n}} \times \vec{E}_{inc} \right)

where

- $\vec{E}_{inc}$ is the incoming electric field vector (set to zero since the wave is generated elsewhere and being absorbed by this boundary),
- $\beta(\mathbf{r})$ is a function containing the condition coefficients (in this case, the wave number of the excited wave $k_0$), and
- $\hat{\mathbf{n}}$ is the boundary normal vector.

A vector penalty boundary condition imposing the condition $E_y = 1$ V/m for both
the real and imaginary components was used to excite the emitted wave.

!alert! warning title=Missing Surface Current Condition
It is important to note that the penalty boundary condition is only used to
excite the electromagnetic wave and determine the radiation intensity pattern of
the antenna for this benchmark. Ideally, a boundary condition specifying the surface
current on the antenna based on a driving voltage would be used here to generate the
physically exact magnitude of the electric field. This *does not* currently exist
within the electromagnetic module, but is a planned addition in the future.
!alert-end!

## Model Parameters

!style halign=left
Important constant model parameters are shown below in [dipole-model-parameters]. Note the following:

- The electric permittivity mentioned above is related to the relative permittivity mentioned below using the relation $\varepsilon = \varepsilon_r \varepsilon_0$ where $\varepsilon_0$ is the vacuum electric permittivity.
- The operating frequency $\omega$ is related to $f$ below using the relation $\omega = 2 \pi f$.

!table id=dipole-model-parameters caption=Constant model parameters for the dipole antenna benchmark study.
| Parameter (unit) | Value(s) |
| - | - |
| Operating frequency, $f$ (GHz) | 1 |
| Relative permittivity, $\varepsilon_r$ (-) | 1 |
| Vacuum electric permittivity, $\varepsilon_0$ (F/m) | $8.854 \times 10^{-12}$ |
| Vacuum magnetic permeability, $\mu_0$ (H/m) | $4 \pi \times 10^{-7}$ |

Calculated parameters needed for the boundary conditions (the wave number in the
direction of propagation of the traveling wave in a 2D waveguide) were calculated
using the following equation (more information in [!citep](griffiths-intro)):

!equation
\beta(\mathbf{r}) = k_0 = \frac{\omega}{c}

where:

- $c$ is the speed of light (or $3 \times 10^8$ m/s).

Using the 1 GHz operating frequency presented in [dipole-results-figure], the value
for $\beta$ (m$^{-1}$) used in the input file shown below is

!equation
\beta = \frac{2 \pi (1 \times 10^9)}{3 \times 10^8}

## Mesh

!style halign=left
The mesh used in this study was created in [Gmsh], using a top-down view of the
geometry shown above in [dipole-geometry-figure]. The `.geo` file used to create
this mesh is shown at the end of this section. To reproduce the corresponding
`.msh` file in a terminal, ensure that gmsh is installed and available in the
system PATH and simply run the following command at the location of
`dipole_antenna_1G.geo`

```
gmsh -2 dipole_antenna_1G.geo -clscale 0.2 -order 2 -algo del2d
```

As of Gmsh 4.6.0, this command produces a second order 2D mesh with an element
size factor of 0.2 (not including the point-wise scaling factors contained
within the mesh file) using a Delaunay algorithm. The unstructured, triangular
mesh contains 39905 nodes and 20157 elements. An image of the result is shown in
[dipole-mesh-figure]. Note that the second order mesh is required because we are using
`NEDELEC_ONE` vector finite elements for all solution variables.

!media dipole_antenna_1G_mesh.png
       style=width:50%;margin:auto;
       id=dipole-mesh-figure
       caption=Mesh used in the dipole antenna benchmark study.

## Mesh File

!listing dipole_antenna_1G.geo

## Far-field Radiation Intensity Theory

!style halign=left
To confirm that the module benchmark is performing as intended, it will be compared
to the far-field radiation intensity pattern for a half-wave dipole antenna. The
far-field electric field for a half-wave antenna is given in [!citep](silver) as

!equation
E_{\theta} = \frac{\eta_0 I_0}{2 \pi r} \frac{\cos(\frac{\pi}{2} \cos\theta)}{\sin \theta} \sin(\omega t - k r)

where

- $\eta_0$ is the impedance of free space (377 $\Omega$),
- $I_0$ is the peak current amplitude in Amps,
- $r$ is the radial position in meters,
- $\theta$ is the angle of position around the antenna relative to one of the end nodes in radians,
- $\omega$ is the driving frequency in Hz, and
- $k$ is the wavenumber in m$^{-1}$.

Total power in the dipole is proportional to $\left| E_{\theta} \right|^2$,
which also means that, directionally, total power is also

!equation
P_{\text{total}} \sim \frac{\cos(\frac{\pi}{2} \cos\theta)}{\sin \theta}

A two-dimensional polar plot of this relationship is shown in [dipole-pattern-figure]
for a vertically-oriented half-wave dipole antenna. This shows that a half-wave
dipole antenna has null, or zero intensity, regions in either direction parallel
to its length, with maximum intensity at $\theta = 90^{\circ}$ and $\theta = 270^{\circ}$.
The beamwidth, or the region outside of which has a signal which is below 3 dB,
or half power, compared to the peak intensity, is around $90^{\circ}$.

!media dipole_radiation_pattern.png
       style=width:50%;margin:auto;
       id=dipole-pattern-figure
       caption=Theoretical radiation intensity pattern of a vertically-oriented half-wave dipole antenna.

## Input File

!listing dipole.i

## Solution and Discussion

!style halign=left
Using the Helmholtz wave equation for vector fields in the frequency domain
discussed above, an exciting complex electric field ($|E_y| = \sqrt{2}$ V/m) was
applied to the surface of the antenna and the radiation field was simulated using
the electromagnetics module. These results are shown in [dipole-results-figure].
Very good qualitative agreement is seen with the theoretical far-field radiation
pattern.

!media dipole_1G_frequency.png
       style=width:50%;margin:auto;
       id=dipole-results-figure
       caption=Electric field radiation pattern of half-wave dipole antenna driven by a 1GHz signal, simulated using the EM module. Note that field intensity is normalized to 1.0.
