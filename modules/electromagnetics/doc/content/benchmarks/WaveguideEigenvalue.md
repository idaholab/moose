# Waveguide Eigenvalue Benchmark

A discussion of eigenvalue problems in electromagnetics, and the basis for the
modeling in this section can be found in [!citep](nasa-fem-eigenvalue-problems).
In this set of verification exercises, fundamental wavenumbers for various waveguide
geometries will be calculated based on that discussion and compared to the
results shown using the HELM10 FEM code developed by Reddy et al.

## Calculation of the Fundamental Eigenvalue and Associated Field Distribution

!style halign=left
Eigenvalue calculation in MOOSE is handled via the SLEPc [!citep](slepc-paper)
package, which is built within PETSc. Using SLEPc, a selection of the largest or
smallest (real and imaginary), closest to a target component (real and imaginary)
or magnitude, or all available eigenvalues within a given problem can be obtained.
Whether more than one eigenvalue can be calculated in a given problem is both a
consequence of the problem being considered (in the case of the waveguides, whether
the geometry can handle more than one mode) and the SLEPc solver options, the nuances
of which are discussed in great detail on the [SLEPc webpage](https://slepc.upv.es/).
The examples presented in this section will all use the SLEPc default, which is to
calculate the smallest real eigenvalue.

The physical equation being solved for is a Helmholtz-style equation for a scalar
potential, given as:

!equation id=eigen-helmholtz
\frac{\partial^2 \psi}{\partial x^2} + \frac{\partial^2 \psi}{\partial y^2} + k_c^2 \psi = 0

where $k_c$ is the eigenvalue and $\psi$ is the associated eigenfunction.
For a transverse electric (TE) mode, $\psi = H_z$, and for a transverse magnetic
(TM) mode, $\psi = E_z$. As previously stated, several waveguide geometries
(rectangular, circular, and coaxial) will be simulated to determine the fundamental
TM mode wavenumber that can exist in each one. In TM mode simulations, the scalar
potential is set to zero on waveguide perfectly conducting walls in order to satisfy
the conditions on $E_z$. Given $E_z$ one can also find expressions for $E_x$ and $E_y$:

!equation
E_x = -Z_0^{\text{TM}} \frac{\partial \psi}{\partial x}

!equation
E_y = -Z_0^{\text{TM}} \frac{\partial \psi}{\partial y}

where $Z_0^{\text{TM}}$ is the characteristic wave impedance for the TM
mode in the waveguide. These expressions were are given in [!citep](harrington-eigenvalues),
where Harrington showed that the transverse electric field distributions could be
determined from a given eigenfunction that satisfies [eigen-helmholtz]. Summary
figures of the geometries are shown in the  following sections, with information
on the sizing and mesh resolution used in each case.

## Model Geometries and Meshes

### Rectangular

!style halign=left
For the simulation results from HELM10, Reddy specified the sizing relationship
for the geometry in [rectangular-geo] of $a_r/b_r = 2$. In the EM module
simulations, $a_r = 2$ for simplicity. The mesh was made up of 2500 structured
triangular elements, generated using the MOOSE mesh system.

!media rectangular_geo.png
       style=width:50%;margin:auto;
       caption=Rectangular waveguide geometry, from [!citep](nasa-fem-eigenvalue-problems).
       id=rectangular-geo

### Circular

!style halign=left
For the simulation results from HELM10, Reddy specified a radius of unity for the
geometry shown in [circular-geo]. The EM module calculation used a mesh of 417
unstructured triangular elements generated using [Gmsh] 4.6.0 with a scaling factor
of 0.5 using a Delaunay triangulation algorithm.

!media circular_geo.png
       style=width:50%;margin:auto;
       caption=Circular waveguide geometry, from [!citep](nasa-fem-eigenvalue-problems).
       id=circular-geo

With Gmsh installed and available in the system PATH, simply run the following
command at the location of `circle.geo`:

```
gmsh -2 circle.geo -clscale 0.5 -order 1 -algo del2d
```

The `.geo` file for this geometry is shown below:

!listing circle.geo

and this generates the following mesh:

!media circle_mesh.png
       style=width:50%;margin:auto;
       id=circle-mesh-figure
       caption=Mesh used in the circle case of the electromagnetic eigenvalue benchmark study.

### Coaxial

!style halign=left
For the simulations results from HELM10, Reddy specified a sizing relationship
$r_2/r_1 = 4$ for the geometry shown in [coaxial-geo]. In the EM module
simulations, $r_1 = 0.125$, and the mesh had 626 unstructured triangular elements
generated using [Gmsh] 4.6.0 with a scaling factor of 0.4 using a Delaunay triangulation algorithm.

!media coaxial_geo.png
       style=width:50%;margin:auto;
       caption=Coaxial waveguide geometry, from [!citep](nasa-fem-eigenvalue-problems).
       id=coaxial-geo

With Gmsh installed and available in the system PATH, simply run the following
command at the location of `coaxial.geo`:

```
gmsh -2 coaxial.geo -clscale 0.4 -order 1 -algo del2d
```

The `.geo` file for this geometry is shown below:

!listing coaxial.geo

and this generates the following mesh file:

!media coaxial_mesh.png
       style=width:50%;margin:auto;
       id=coaxial-mesh-figure
       caption=Mesh used in the coaxial case of the electromagnetic eigenvalue benchmark study.

## Input File

A base input file was used to run each case in this benchmark.

!listing eigen_base.i

In order to simulate the right case, the following command line arguments should be applied to the input file at runtime:

### Rectangular

```
Outputs/file_base=eigen_rectangular_out
```

### Circle

```
Outputs/file_base=eigen_circular_out BCs/active="circle eigen_circle" Mesh/file=circle.msh
```

### Coaxial

```
Outputs/file_base=eigen_coaxial_out BCs/active="coaxial eigen_coaxial" Mesh/file=coaxial.msh
```

## Results and Discussion

Calculated cutoff wavenumbers using the module for the fundamental TM mode in each
geometry are presented in [eigen-wavenumber-results]. Results are in excellent
agreement with the analytical solution presented by Harrington and the results
calculated by HELM10, with at most 0.68% relative error.

!table id=eigen-wavenumber-results caption=Cutoff wavenumbers for various waveguide geometries and modes.
| Shape | TM | Quantity | Analytical [!citep](harrington-eigenvalues) | EM Module | HELM10 [!citep](nasa-fem-eigenvalue-problems) |
| - | - | - | - | - | - |
| Rectangular | 11 | $k_c a_r$ | 7.027 | 7.032 | 7.027 |
| Circular | 01 | $k_c r$ | 2.405 | 2.408 | 2.413 |
| Coaxial | 01 | $k_c r_1$ | 1.024 | 1.031 | 1.030 |

Reddy presents field distributions as glyph vector fields rather than discrete
contours, so a qualitative comparison is performed for field distribution
verification. Comparisons for each geometry case are shown in [rectangular-results-emm]
and [rectangular-results-reddy] for the rectangular geometry, [circular-results-emm]
and [circular-results-reddy] for the circular geometry, and [coaxial-results-emm]
and [coaxial-results-reddy] for the coaxial geometry. Overall, results are in
general in good agreement with that of HELM10, in terms of vector field orientation,
except for the circular TM01 waveguide case. In this instance, potential distribution
was identical except for the sign, where the EM module predicted a positive peak
in the center of the waveguide, and HELM10 predicted a negative peak. This difference
may be the result of the module wave directionality assumption (both assuming
$z$-direction, but might be differing in sign). This bears out in a comparison
with the COMSOL package, where the TM01 result for a circular waveguide is shown
below in [comsol-tm01] and both the EM module and COMSOL make the assumption of
a $exp(-jk_z z)$ component, while [!citep](nasa-fem-eigenvalue-problems) isn't
clear on their assumption here. Regardless, either result fits with the generic
expected field distribution presented by Jin in [jin-tm01], where the solid lines
represent the electric field lines. Thus, both HELM10 and the EM module are
consistent with established literature expectations for this waveguide geometry.
Other visual differences in the vector glyph distribution and magnitude in all
cases are due to mesh differences and Paraview visualization software scaling.

!row!
!col small=12 medium=6 large=6
!media rectangleTM11.png
       style=width:100%;margin:auto;
       caption=TM11 mode electric field distribution in a rectangular waveguide, calculated by the EM module.
       id=rectangular-results-emm

!col small=12 medium=6 large=6
!media rectangleTM11_reddy.png
       style=width:100%;margin:auto;
       caption=TM11 mode electric field distribution in a rectangular waveguide, calculated by HELM10. [!citep](nasa-fem-eigenvalue-problems)
       id=rectangular-results-reddy
!row-end!

!row!
!col small=12 medium=6 large=6
!media circleTM01.png
       style=width:100%;margin:auto;
       caption=TM01 mode electric field distribution in a circular waveguide, calculated by the EM module.
       id=circular-results-emm

!col small=12 medium=6 large=6
!media circleTM01_reddy.png
       style=width:100%;margin:auto;
       caption=TM01 mode electric field distribution in a circular waveguide, calculated by HELM10. [!citep](nasa-fem-eigenvalue-problems)
       id=circular-results-reddy
!row-end!

!row!
!col small=12 medium=6 large=6
!media circular_TM01_comsol.png
       style=width:100%;margin:auto;
       caption=TM01 mode electric field distribution in a circular waveguide, calculated by the COMSOL RF Module in [!citep](comsol-circular-port-boundary-condition). (Coloring is the electric field norm)
       id=comsol-tm01

!col small=12 medium=6 large=6
!media circular_TM01_jin.png
       style=width:100%;margin:auto;
       caption=TM01 mode field distribution in a circular waveguide, presented in [!citep](jin-computation). Note that the solid lines correspond to the electric field.
       id=jin-tm01
!row-end!

!row!
!col small=12 medium=6 large=6
!media coaxialTM01.png
       style=width:100%;margin:auto;
       caption=TM01 mode electric field distribution in a coaxial waveguide, calculated by the EM module.
       id=coaxial-results-emm

!col small=12 medium=6 large=6
!media coaxialTM01_reddy.png
       style=width:100%;margin:auto;
       caption=TM01 mode electric field distribution in a coaxial waveguide, calculated by HELM10. [!citep](nasa-fem-eigenvalue-problems)
       id=coaxial-results-reddy
!row-end!
