# Waveguide Transmission Benchmark

This section summarizes and describes the single frequency two-dimensional
waveguide benchmark and verification test for the electromagnetics module.

## Model Geometry

!style halign=left
In this scenario, the geometry is a vacuum-filled waveguide of length $L$ and width
$b$, as shown in [waveguide-benchmark-geometry]. In this waveguide,
an electromagnetic plane wave representing the TM11 mode travels from the left
entry port (x = 0) to the right exit port (x = L). Parameters for this scenario
are shown in [waveguide-benchmark-parameters].

!media waveguide.png
       style=width:100%;margin:auto;
       id=waveguide-benchmark-geometry
       caption=Two-dimensional vacuum-filled waveguide geometry, with an incoming plane wave at left and exiting at right.

!table id=waveguide-benchmark-parameters caption=Model parameters for the waveguide benchmark study.
| Parameter (unit) | Value(s) |
| - | - |
| Waveguide length, $L$ (m) | 80 |
| Waveguide width, $b$ (m) | 10 |
| Operating frequency, $f$ (MHz) | 20 |
| Vacuum electric permittivity, $\varepsilon_0$ (F/m) | $8.854 \times 10^{-12}$ |
| Vacuum magnetic permeability, $\mu_0$ (H/m) | $4 \pi \times 10^{-7}$ |

## Governing Equations and Boundary Conditions

!style halign=left
In this simulation, both the real and imaginary components of the electric field
wave will be simulated separately, but the frequency-domain electric field wave
equation in general is given by:

!equation
\nabla \times \nabla \times \vec{E} - \mu_0 \varepsilon \omega^2 \vec{E} = 0

where

- $\vec{E}$ is the complex electric field vector in V/m,
- $\mu_0$ is the vacuum magnetic permeability of the medium in H/m,
- $\varepsilon$ is the electric permittivity of the medium in F/m, and
- $\omega$ is the operating frequency in rad/s.

Harrington in [!citep](harrington-eigenvalues) showed that the transverse electric
field distributions could be determined from the component in the direction of
wave travel, so only the $E_x$ scalar component will need to be modeled for
comparison. Further, because the real and imaginary components of the field are
90$^{\circ}$ out of phase with each other, verifying both the form of the real
component and the phase shift should be adequate to check for proper performance.

Given Gauss's Law with an absence of charge density (as the waveguide is filled
with vacuum), a standard diffusion-reaction style equation (without a source) is
the final form of the equation being modeled, with the [Diffusion.md] and
[ADMatReaction.md] objects being used for each real and imaginary component.

With respect to boundary conditions, $E_x$ is set to zero on the waveguide walls,
to satisfy the perfect electrical conductor boundary condition for this case. For
the entry port, the [EMRobinBC.md] object will be used in a "port" configuration,
while the exit port will be set in an "absorbing" configuration. The incoming wave
shape across the waveguide port needed by [EMRobinBC.md] will be set to

!equation
E_{\text{incoming}} = \sin \left( \frac{\pi y}{10} \right)

## Analytic Solution for Comparison

!style halign=left
In order to perform a quantitative comparison, the instantaneous field expressions
for a rectangular waveguide are needed. Cheng provided real field component
expressions for the TM11 mode in a three-dimensional rectangular waveguide in
[!citep](cheng), and the $E_x$ component is as follows for this scenario:

!equation id=cheng-solution
E_x = E_0 \sin \left( \frac{\pi z}{a} \right) \sin\left( \frac{\pi y}{b} \right) \cos(\omega t - \beta x)

where

- $h^2 = (\pi / a)^2 + (\pi / b)^2$ (or just $(\pi / a)^2$ for a 2-D case),
- $\beta = \sqrt{k^2 - h^2}$ is the propagation constant of the wave,
- $k = \omega / c$ is the free space wavenumber, with $c$ being the speed of light,
- $\omega = 2 \pi f$ is the operating frequency in rad/s,
- $E_0$ is the peak electric field amplitude,
- $a$ is the width of the waveguide in the $z$ direction, and
- $b$ is the width of the waveguide in the $y$ direction.

Because our two-dimensional case is effectively infinite into and out of the page,
the $\sin\left( \frac{\pi z}{a} \right)$ factor can be removed to render the
analytic function for this verification.

## Mesh

!style halign=left
The mesh used in this study was created in [Gmsh], using a top-down view of the
geometry shown above in [waveguide-benchmark-geometry]. The `.geo` file used to
create this mesh is shown at the end of this section. To reproduce the content of
the corresponding `.msh` file (`waveguide.msh`) in a terminal, ensure that
gmsh is installed and available in the system PATH and simply run the following
command at the location of `waveguide.geo`

```
gmsh -2 waveguide.geo -clscale 0.12 -order 1 -algo del2d
```

As of Gmsh 4.6.0, this command produces a first order 2D mesh with an element
size factor of 0.12 using a Delaunay algorithm. The unstructured, triangular
mesh contains 1202 nodes and 2806 elements. An image of the result is shown in
[waveguide-mesh-figure].

!media waveguide2D_mesh.png
       style=width:100%;margin:auto;
       id=waveguide-mesh-figure
       caption=Mesh used in the waveguide transmission benchmark study.

### Mesh File

!listing waveguide.geo

## Input File

!listing waveguide2D_test.i

## Results and Discussion

!style halign=left
The field result for this simulation (real and imaginary components) is shown in
[waveguide-benchmark-real] and [waveguide-benchmark-imaginary].

!media waveguide2D_real.png
       style=width:100%;margin:auto;
       id=waveguide-benchmark-real
       caption=Electric field result, $E_x$ (real component), of the 2-D waveguide verification case.

!media waveguide2D_imaginary.png
       style=width:100%;margin:auto;
       id=waveguide-benchmark-imaginary
       caption=Electric field result, $E_x$ (imaginary component), of the 2-D waveguide verification case.

A comparison of the real component results to compared to that in [cheng-solution]
is shown in [waveguide-benchmark-comparison]. This data is taken down the centerline
of the waveguide geometry ($y = 5$). There is decent agreement between the two
solutions, but note the increasing phase error along the length of the waveguide.
This is known as "numerical disperson" or "numerical phase error" in the literature,
and is a byproduct of the numerical discretization, where the velocity of the
calculated wave differs from that of the exact velocity due to lower resolution.
Jin as well as Warren and Scott showed that the use of higher order elements as
well as smaller elements can help mitigate this effect.
[!citep](jin-fem, jin-computation, warren-and-scott-order) Further, Warren and
Scott later showed that the orientation of the elements in the mesh for a triangular
mesh, as used here, had a notable impact on the calculated phase error.
[!citep](warren-and-scott-mesh) Since the error accumulates in the direction of
wave travel, alternating the direction of the triangular elements in a hexagonal
configuration had the lowest phase error in a TM1 mode simulation. Interestingly,
randomizing the triangular element direction did not have the same impact (though
did achieve lower phase error overall). A recalculation of this result using a
finer mesh is shown in [waveguide-benchmark-comparison-finer], and shows that
the refined (18,102 elements compared to 2,219 in the original), simulated result
is almost exactly in alignment with the analytic solution.

!media waveguide2D_comparison.png
       style=width:100%;margin:auto;
       id=waveguide-benchmark-comparison
       caption=Comparison of the real electric field result with the analytic solution in the 2-D waveguide verification case.

!media waveguide2D_comparison_finer.png
       style=width:100%;margin:auto;
       id=waveguide-benchmark-comparison-finer
       caption=Comparison of the real electric field result with the analytic solution in the 2-D waveguide verification case, with a refined mesh.

Finally to confirm the proper resolution of the phase between real and imaginary
components (and to show that the port and absorbing boundary conditions have little
reflection), a plot of the magnitude of the electric field is shown below in
[waveguide-benchmark-magnitude]. If the components are 90$^{\circ}$ out-of-phase
with each other, then the magnitude of the field should be a
$\sin \left( \frac{\pi y}{10} \right)$ profile, extending the length of the
waveguide. Indeed, [waveguide-benchmark-magnitude] confirms that this is the case.

!media waveguide2D_magnitude.png
       style=width:100%;margin:auto;
       id=waveguide-benchmark-magnitude
       caption=Electric field result for the magnitude of $E_x$ in the 2-D waveguide verification case, confirming that the real and imaginary components are in the proper phase relative to each other.
