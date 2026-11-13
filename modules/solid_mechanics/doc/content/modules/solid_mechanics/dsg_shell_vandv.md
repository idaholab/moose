# DSG shell elements: verification

The cases below are the inputs of `modules/solid_mechanics/test/tests/shell_dsg`, set up as described in [DSGShellElements](/DSGShellElements.md). Every value is compared with a closed-form or a published reference. The test specifications run each input as it is written; the rows marked *off-line* were obtained from the same inputs with the mesh, boundary or member options changed on the command line, and are not regression tests. Each case uses one consistent unit system, quoted as in its input. Where a case runs on the four members of the family, the member is given; Q4 is the four-node quadrilateral, T3 the three-node triangle, T6 the six-node triangle and Q8 the eight-node quadrilateral.

## Cantilever strip in bending

A strip of 10 m by 1 m, thickness 0.1 m, E = 2.1e6 and $\nu$ = 0, clamped at one end and loaded by P = 1 N at the other, with four elements along its length (the problem of Figure 6a of [!cite](dvorkin1984continuum)).

!listing modules/solid_mechanics/test/tests/shell_dsg/static/cantilever_strip.i block=Mesh BCs NodalKernels

The analytic tip deflection of the shear-deformable cantilever is $P L^3 / (3 E I) + P L / (\kappa A G) = 1.904876$ m with $\kappa = 5/6$, the tip slope $P L^2 / (2 E I) = 0.2857143$, and the bending moment at the center of the first quadrilateral, 1.25 m from the clamp, $P (L - 1.25) = 8.75$ N m per unit width; it is reported negative, the loaded face being in compression. The elemental values of the triangles are averages over a different area (the first six-node triangle has its centroid 0.83 m from the clamp, where the moment is 9.17) and are listed without a ratio.

| Member | Tip deflection (m) | Ratio | Tip slope | Ratio | Moment, first element | Ratio | Shear, first element |
|---|---|---|---|---|---|---|---|
| Q4 | 1.875114 | 0.9844 | 0.2857143 | 1.0000 | -8.7500 | 1.0000 | 1.0000 |
| T3 | 1.893993 | 0.9943 | 0.2888268 | 1.0109 | -8.7361 | - | 1.0717 |
| T6 | 1.905462 | 1.0003 | 0.2866637 | 1.0033 | -9.1667 | - | 1.0000 |
| Q8 | 1.904979 | 1.0001 | 0.2857336 | 1.0001 | -8.7500 | 1.0000 | 1.0000 |

The four-node shell of the module gives 1.875095 m for the same case.

The same strip rotated to the XZ plane (the rotated_beam_bending case of the module) gives the same tip deflection, 1.875114 m, and the same slope, moment and shear in magnitude; the signs of the latter follow the element frame, whose normal now points along $-Y$.

## Simply supported square plate under a uniform load

One quarter of a 50 m square plate of thickness 1 m, E = 1e9 and $\nu$ = 0.3, under q = 10.816 N/m$^2$. The Navier series of [!cite](timoshenko1959theory) gives the thin-plate center deflection 2.998536e-3 m. The shear deformation of this plate ($t / a = 1 / 50$) adds 0.22 percent, so the converged value of the shear-deformable plate with hard simple supports is 3.0050e-3 m.

!listing modules/solid_mechanics/test/tests/shell_dsg/static/plate_bending_distributed.i block=BCs Kernels/pressure

| Case | Member | Mesh of the quarter | Center deflection (m) | Ratio to 3.0050e-3 |
|---|---|---|---|---|
| nodal forces, soft supports (the module's plate_bending case) | Q4 | 25 x 25 | 3.002819e-3 | 0.9993 |
| distributed load, hard supports | Q4 | 10 x 10 | 3.002601e-3 | 0.9992 |
| distributed load, hard supports | T3 | 10 x 10 | 2.980827e-3 | 0.9920 |
| distributed load, hard supports | T6 | 10 x 10 | 3.005042e-3 | 1.0000 |
| distributed load, hard supports | Q8 | 10 x 10 | 3.005008e-3 | 1.0000 |
| distributed load, hard supports, *off-line* | Q8 | 20 x 20 | 3.005011e-3 | 1.0000 |
| distributed load, Gmsh mesh (plate_bending_gmsh case) | Q4 | 5 x 5 | 3.006139e-3 | 1.0004 |

With soft simple supports (transverse displacement only; *off-line*, Q8 on 20 x 20) the quadratic members converge instead to 3.053e-3 m, 1.6 percent above the thin-plate value: the boundary layer of the Reissner-Mindlin plate at a soft support, which the coarse four-node mesh does not resolve.

### Thin plate on a distorted mesh

The four-node element has no thin-limit factor, so its freedom from shear locking is checked on the same plate with the thickness reduced to 0.05 m ($t / a = 1 / 1000$) and the load scaled with the thickness cubed, q = 1.352e-3 N/m$^2$, which keeps the thin-plate center deflection at 2.9971e-3 m (the shear deformation adds 0.0002 percent at this slenderness). The interior nodes of the 10 x 10 mesh are moved by up to 0.3 m with a sine-cosine field, so no element is a rectangle; a displacement-based element would be rigid on this plate.

!listing modules/solid_mechanics/test/tests/shell_dsg/static/plate_bending_thin_distorted.i block=Mesh

| Mesh of the quarter | Center deflection (m) | Ratio to 2.9971e-3 |
|---|---|---|
| 10 x 10 regular | 2.99639e-3 | 0.9998 |
| 10 x 10 distorted | 2.99617e-3 | 0.9997 |
| 20 x 20 distorted | 2.99774e-3 | 1.0002 |

The distorted 10 x 10 mesh is the test; the other two rows were run with the same input and the distortion or the mesh size changed.

## Plate with force and moment loads

The single-element plate of problem 1.3.13 of the Abaqus verification manual (case plate_force_moment_loads), 40 m by 20 m by 1 m, E = 1000 Pa and $\nu$ = 0.3, with a vertex force and edge moments. The deflection at the loaded vertex is 12.527 m against 12.54 m with the S4 element, a ratio of 0.999.

## Cantilever plate under an end rotation

A 10 m by 1 m plate, E = 2e11 and thickness 0.133887 m ($I$ = 2e-4 m$^4$ per unit width), under a rotation of 0.05 at its free end: the bending moment is constant, $E I \theta / L$ = 2e5 N m per unit width, and the tip deflection $M L^2 / (2 E I)$ = 0.25 m. The element gives both to seven digits and a transverse shear force of 3e-7, zero within round-off.

## Simply supported strip under two line loads

A 9 m by 1 m plate of E = 200e9 and thickness 0.133887 m, simply supported at both ends, under two line loads of 10000 N/m at 3 m and 6 m.

| Quantity | Beam theory | Q4 on the Gmsh mesh |
|---|---|---|
| center deflection (m) | 6.469e-3 | 6.436e-3 |
| center bending moment (N m / m) | 30000 | 30011 |
| shear force, first span | 10000 | 9935 |
| shear force, middle span | 0 | -181 |
| shear force, last span | -10000 | -10115 |

## Pinched cylinder

The cylinder of radius 1 m and length 2 m with rigid end diaphragms, pinched by two opposite loads of 10 N at mid-length, E = 1e6, $\nu$ = 0.3 and thickness 0.01 m (Figure 10 and Table 6 of [!cite](dvorkin1984continuum)). One eighth is modeled with a 10 x 10 mesh mapped onto the cylinder. The series solution gives a displacement under the load of 0.16424 m.

!listing modules/solid_mechanics/test/tests/shell_dsg/static/pinched_cylinder.i block=Mesh

| Member | Mesh | Displacement under the load (m) | Ratio |
|---|---|---|---|
| Q4 | 10 x 10 | 0.133656 | 0.814 |
| Q4, unstructured mesh (the module's pinched_cylinder_symm_unstructured mesh, 108 elements) | unstructured | 0.121679 | 0.741 |

The module's four-node shell gives 0.82 on its 10 x 10 mesh and [!cite](dvorkin1984continuum) report 0.83. The load applied along the other symmetry plane gives the same displacement to all digits, and so does the structured mesh with the first local axis of the elements set to $(1, -1, 0)$ by `reference_first_local_direction`: the displacements do not depend on the element frame.

### Drilling stiffness factor

The drilling stiffness ties the drilling rotation to the in-plane displacement field and stiffens a membrane-dominated problem on a coarse mesh. The 10 x 10 pinched cylinder with `drilling_stiffness_factor` reduced gives:

| Factor | Displacement under the load (m) | Ratio |
|---|---|---|
| 1 (default) | 0.133656 | 0.814 |
| 0.1 | 0.135095 | 0.823 |
| 0.01 | 0.135261 | 0.824 |

A factor of 0.1 recovers one percent of the displacement and a further reduction changes nothing: the stiffness of the coarse mesh comes from the membrane interpolation, not from the drilling penalty. The factor 0.1 is a test; the last row was run off line.

## Cylindrical tank under internal pressure

The tank of the module's tank_shell case: radius 0.5 m, length 3 m, thickness 0.03 m, E = 2e5 MPa and $\nu$ = 0.3, held at its lower circle and under an internal pressure of 40 MPa applied against the element normals by [DistributedLoadDSGShell](/DistributedLoadDSGShell.md) (the normals of this mesh point into the tank, so a positive pressure acts outward); the upper circle is free. In the membrane state away from the base the hoop force is $p r$ = 20 MN/m, independent of the material, the radial displacement $p r^2 / (E t)$ = 1.6667e-3 m and the axial force zero.

!listing modules/solid_mechanics/test/tests/shell_dsg/static/tank_shell.i block=Kernels/pressure_disp_x Materials/strain

| Case | Hoop force at mid-height (MN/m) | Ratio | Radial displacement (m) | Ratio | Axial force |
|---|---|---|---|---|---|
| upright tank | 19.728 | 0.986 | 1.6439e-3 | 0.986 | 2e-13 |
| tank inclined at 45 degrees | 19.727 | 0.986 | 1.6439e-3 | 0.986 | 2e-13 |

The 1.4 percent deficit is the one of the faceted geometry: the mesh has 20 flat elements around the circumference, and a pressure on flat facets of a 20-sided polygon produces the hoop force of a cylinder of a slightly smaller radius. The inclined tank gives the same values to five digits.

## Scordelis-Lo roof

The cylindrical roof of [!cite](macneal1985patch), radius 25 m, length 50 m, 80 degrees of opening, E = 4.32e8, $\nu$ = 0 and thickness 0.25 m, under its self weight of 90 per unit area, with rigid diaphragms at its curved ends and free straight edges. One half is modeled on the 15 x 15 structured mesh of the module's shell battery.

| Member | Vertical deflection of the free edge midpoint (m) | Reference | Ratio |
|---|---|---|---|
| Q4 | 0.30870 | 0.3086 | 1.0003 |

## Membrane states

A single 1 mm square element of E = 5 N/mm$^2$, $\nu$ = 0 and thickness 0.1 mm stretched by 0.2 mm gives the membrane force $E t \varepsilon$ = 0.1 N/mm exactly and zero for the other resultants. The same element rotated by 45 degrees about the X axis and stretched along its inclined axis gives the same 0.1 N/mm in its second local direction and zero elsewhere: the resultants are reported in the element frame and do not depend on the orientation of the element in space.

## Free vibration of a cantilever plate

A plate of 1.0 m by 1.5 m, E = 2.1e6, $\nu$ = 0.3, thickness 0.1 and density 1, clamped along one short edge, is set vibrating by a short force pulse on its free edge and left to vibrate freely with the Newmark time integrator ($\beta$ = 0.25, $\gamma$ = 0.5, $\Delta t$ = 0.005 s). The specifications run one element and ten or one hundred steps as regression checks; the frequencies below are *off-line* runs of the same input on 4 x 8 elements for 1 s, the fundamental frequency read from the zero crossings of the tip displacement after the pulse. The thin-plate reference of [!cite](blevins2016formulas), Table 5.3 case 11, is $f = \lambda^2 / (2 \pi a^2) \sqrt{E h^2 / (12 \rho (1 - \nu^2))}$ = 10.785 Hz for this plate, with $a$ = 1.5 m the clamped-to-free length and $\lambda^2$ = 3.477; it neglects the shear deformation and the rotary inertia, both of which lower the frequency of a plate of this thickness ($t / a$ = 1 / 15).

!listing modules/solid_mechanics/test/tests/shell_dsg/dynamics/free_vibration.i block=Executioner

| Member | Mesh (*off-line*) | Frequency (Hz) | Ratio to the thin-plate value |
|---|---|---|---|
| Q4 | 4 x 8 | 10.587 | 0.982 |
| T3 | 4 x 8 | 10.525 | 0.976 |
| T6 | 4 x 8 | 10.546 | 0.978 |
| Q8 | 4 x 8 | 10.547 | 0.978 |
| Q4, rotary inertia off | 4 x 8 | 10.597 | 0.983 |

The two quadratic members agree to 0.01 percent. The module's four-node shell gives 10.612 Hz with eight elements along the length. These transient frequencies sit 0.9 percent below the eigenvalues of the same meshes in the next section: the period elongation of the Newmark scheme at this time step, $(\omega \Delta t)^2 / 12$, is 0.9 percent.

## Natural frequencies

The same plate, 4 x 8 elements, with the mass written for the [Eigenvalue](/Eigenvalue.md) executioner ([DSGShellMassEigenKernel](/DSGShellMassEigenKernel.md)), three eigenpairs by shift-and-invert Krylov-Schur.

!listing modules/solid_mechanics/test/tests/shell_dsg/dynamics/natural_frequencies.i block=Kernels/inertia_rot_x BCs/eigen_clamp_disp_z Executioner

| Member | $\omega_1^2$ (s$^{-2}$) | $f_1$ (Hz) | Ratio to the thin-plate value | $\omega_2^2$ | $\omega_3^2$ |
|---|---|---|---|---|---|
| Q4 | 4508.24 | 10.686 | 0.991 | 49365 | 179214 |
| Q4, rotary inertia off | 4516.06 | 10.695 | 0.992 | 49904 | 181427 |
| T3, *off-line* | 4456.63 | 10.625 | 0.985 | 46841 | 162907 |
| T6 | 4475.29 | 10.647 | 0.987 | 47564 | 165945 |
| Q8, *off-line* | 4475.46 | 10.647 | 0.987 | 47766 | 166058 |

The quadratic members again agree with each other, at 1.3 percent below the thin-plate value, the shear deformation and the rotary inertia of a plate of this thickness. The rotary inertia lowers the first frequency of the four-node element by 0.1 percent and the third by 1.2 percent.

## Resultants at the nodes

The cantilever strip of the first section, with its bending moment, $-P (L - y)$ in the sign convention of the family, recovered at the nodes by [DSGShellNodalResultants](/DSGShellNodalResultants.md): each element evaluates the moment at its own nodes and the nodal value is the average over the elements sharing the node.

| Node | Exact | Q4 | Q8 |
|---|---|---|---|
| clamp, y = 0 | -10.00 | -8.75 | -10.000 |
| y = 2.5 | -7.50 | -7.50 | -7.500 |
| y = 5 | -5.00 | -5.00 | -5.001 |
| y = 7.5 | -2.50 | -2.50 | -2.501 |
| tip, y = 10 | 0.00 | -1.25 | 0.001 |

On the four-node element the curvature is constant along the strip, so a node between two elements gets the mean of their center moments, exact for a linear moment, and the two end nodes get the center moment of their single element. On the eight-node element the moment is linear within the element and every node is exact to 0.05 percent.

## Jacobian

The Jacobian of every kernel is compared with a finite difference of the residual by the `PetscJacobianTester` specs of the battery, on the four members for the static kernels and on the four-node element for the inertia kernel with its rotary inertia. The element is linear, so the comparison holds to the precision of the finite difference.

!bibtex bibliography
