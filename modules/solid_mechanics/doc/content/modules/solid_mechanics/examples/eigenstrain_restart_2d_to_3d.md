# Eigenstrain Restart from 2D Axisymmetric to 3D

## Overview

This tutorial demonstrates how to use axisymmetric simulations as a basis for 3D mechanical analyses.
The workflow consists of two steps:

1. Perform a 2D axisymmetric plasticity simulation and save the resulting plastic eigenstrain.
2. Map that eigenstrain onto a 3D mesh and equilibrate the stress field using the [Axisymmetric2D3DSolutionFunction.md] tensor mode.

This approach leverages the lower computational cost of 2D axisymmetric analysis while preserving the
full 3D stress-strain state in the subsequent model.

## Section 1: 2D Plasticity and Equilibration Restart in 2D

### Conceptual basis

In small-strain plasticity, the total strain is decomposed additively into elastic and inelastic parts:

\begin{equation}
\vec{\epsilon} = \vec{\epsilon}_e + \vec{\epsilon}_p
\end{equation}

In an elastic-plastic model, the stress depends only on the elastic strain. If we save the plastic
eigenstrain (the inelastic contribution) from one analysis and re-impose it as a fixed, known field
in a fresh elastic simulation, the resulting stress field is identical to the original. This property
allows efficient 2D-to-3D workflow:

- Run a full 2D axisymmetric simulation (including plasticity, time integration, BCs, etc.). Save the
  final plastic eigenstrain and the mesh/solution.
- In a separate 3D analysis, read the saved 2D eigenstrain, rotate it to 3D Cartesian coordinates,
  and impose it as a fixed material property. Solve for the 3D stress using only elasticity.

### Example: simple 2D axisymmetric plasticity

The following input runs a minimal 2D RZ plasticity simulation, saving the plastic strain components
as auxiliary variables for later restart:

!listing modules/solid_mechanics/test/tests/eigenstrain_restart/simple_2d.i start=[Mesh] end=[Materials]

The simulation creates a plastic deformation and saves the plastic eigenstrain field with components
`eig_rr`, `eig_yy`, `eig_ry`, `eig_tt` (corresponding to cylindrical radial-radial, axial-axial,
radial-axial shear, and hoop-hoop components).

### Restart in 2D with fixed eigenstrain

To verify that re-imposing the saved plastic strain recovers the original stress field,
the following input reads the 2D result and enforces the plastic eigenstrain as a fixed material property:

!listing modules/solid_mechanics/test/tests/eigenstrain_restart/restart_3d.i block=Functions

The equilibrated 2D result matches the original stress field (within solver tolerance), confirming the approach.

## Section 2: Mapping 2D Eigenstrain to 3D

### Motivation

Once the 2D plastic eigenstrain is confirmed, the next step is to embed it into a 3D geometry.
The [Axisymmetric2D3DSolutionFunction.md] tensor mode
automates the cylindrical-to-Cartesian rotation that was previously done by hand-written `ParsedFunction` algebra.

### Legacy approach: six hand-written ParsedFunctions

Before the tensor mode was available, users had to manually implement the rotation formulas.
For an axisymmetric (y-axis) geometry with cylindrical components `(r, y, t)`, the Cartesian
components `(x, y, z)` are related by:

\begin{equation}
T_{xx} = \frac{T_{rr} x^2 + T_{\theta\theta} z^2}{r^2}
\end{equation}

\begin{equation}
T_{zz} = \frac{T_{rr} z^2 + T_{\theta\theta} x^2}{r^2}
\end{equation}

\begin{equation}
T_{xz} = \frac{(T_{rr} - T_{\theta\theta}) x z}{r^2}
\end{equation}

\begin{equation}
T_{xy} = \frac{T_{ry} x}{\sqrt{x^2 + z^2}}
\end{equation}

\begin{equation}
T_{yz} = \frac{T_{ry} z}{\sqrt{x^2 + z^2}}
\end{equation}

\begin{equation}
T_{yy}^{\text{cart}} = T_{yy}^{\text{cyl}}
\end{equation}

where `r = sqrt(x^2 + z^2)` is the distance from the y-axis.

The legacy input file implements these as six `ParsedFunction` blocks:

```
[Functions]
  # Read the four independent axisymmetric components
  [e_rr]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'reconst_eigenstrain_00'
  []
  [e_ry]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'reconst_eigenstrain_01'
  []
  [e_yy_axi]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'reconst_eigenstrain_11'
  []
  [e_tt]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'reconst_eigenstrain_22'
  []

  # Cylindrical -> Cartesian rotation via ParsedFunctions
  [e_xx]
    type = ParsedFunction
    expression = '(err*x*x + ett*z*z)/(x*x + z*z + eps)'
    symbol_names = 'err ett eps'
    symbol_values = 'e_rr e_tt 1e-16'
  []
  [e_zz]
    type = ParsedFunction
    expression = '(err*z*z + ett*x*x)/(x*x + z*z + eps)'
    symbol_names = 'err ett eps'
    symbol_values = 'e_rr e_tt 1e-16'
  []
  [e_xz]
    type = ParsedFunction
    expression = '((err - ett)*x*z)/(x*x + z*z + eps)'
    symbol_names = 'err ett eps'
    symbol_values = 'e_rr e_tt 1e-16'
  []
  [e_xy]
    type = ParsedFunction
    expression = 'ery*x/sqrt(x*x + z*z + eps)'
    symbol_names = 'ery eps'
    symbol_values = 'e_ry 1e-16'
  []
  [e_yz]
    type = ParsedFunction
    expression = 'ery*z/sqrt(x*x + z*z + eps)'
    symbol_names = 'ery eps'
    symbol_values = 'e_ry 1e-16'
  []
  [e_yy]
    type = ParsedFunction
    expression = 'eyy'
    symbol_names = 'eyy'
    symbol_values = 'e_yy_axi'
  []
[]
```

### Modern approach: Axisymmetric2D3DSolutionFunction tensor mode

The new tensor mode internalizes the rotation, eliminating the need for hand-written algebra:

!listing modules/solid_mechanics/test/tests/eigenstrain_restart/restart_3d.i block=Functions

This is equivalent to the legacy block but more concise and less error-prone. The function automatically
computes the rotation matrix from the 3D point and the axis-of-symmetry specification, applies the
cylindrical-to-Cartesian transformation, and extracts the requested Cartesian component.

### 3D restart input

The full 3D restart simulation uses the tensor-mode functions and a GenericFunctionRankTwoTensor
to assemble the six Cartesian components:

!listing modules/solid_mechanics/test/tests/eigenstrain_restart/restart_3d.i block=Materials

The mesh is generated in-input via mesh generators (no external `.e` file). The boundary conditions
and constitutive model are purely elastic. The eigenstrain material property assembles the six
Cartesian tensor components from the six `Axisymmetric2D3DSolutionFunction` instances, and the
solver computes the equilibrium stress field.

### Verification

The equilibrated 3D stress field should reproduce the rotated 2D reference. On a 3D slice plane
aligned with the original 2D mesh, the `max_principal_stress` should match within solver tolerance.

## References

See the [Axisymmetric2D3DSolutionFunction.md] class
documentation for details on the tensor mode, on-axis requirements, and axis-override parameters.
