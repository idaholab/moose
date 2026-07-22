# Kinematics

## Overview id=overview

The Solid Mechanics module describes a mechanics problem with three cooperating pieces:

1. a *kinematics* (strain) calculator, [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md),
   which turns the displacement field into deformation measures;
2. a *constitutive* (stress) calculator, a model deriving from
   [`ComputeLagrangianStressCauchy`](ComputeLagrangianStressCauchy.md) or
   [`ComputeLagrangianStressPK1`](ComputeLagrangianStressPK1.md), which maps those deformation
   measures to stress; and
3. a *stress-divergence* kernel, [`TotalLagrangianStressDivergence`](TotalLagrangianStressDivergence.md)
   or [`UpdatedLagrangianStressDivergence`](UpdatedLagrangianStressDivergence.md), which enforces the
   balance of linear momentum [!cite](10.1145/3716308).

This page covers the first piece, the kinematics.  Splitting the problem this way means a single
strain calculator supplies a common set of deformation measures that *any* constitutive model can
consume, and the same model works with either the total or updated Lagrangian kernel.

Each of the three objects writes material properties that the others read by name.  When more than
one mechanics model runs on the same block, prefix the properties with the `base_name` parameter so
the strain calculator, stress calculator, and kernel that belong together share a name and stay
decoupled from any other set.

## Reference and Current Configurations id=configurations

The kinematics relate two descriptions of the body: the *reference* configuration $\Omega_0$ with
coordinates $\boldsymbol{X}$, and the *current* (deformed) configuration $\Omega$ with coordinates
$\boldsymbol{x}$.  Following the usual convention, upper-case indices denote quantities in the
reference configuration and lower-case indices quantities in the current configuration.  A gradient
with respect to the current coordinates is written
\begin{equation}
      f_{i,j}=\frac{\partial f_{i}}{\partial x_{j}}
\end{equation}
and a gradient with respect to the reference coordinates
\begin{equation}
      f_{i,J}=\frac{\partial f_{i}}{\partial X_{J}} .
\end{equation}
The map between the configurations is characterized by the deformation gradient
\begin{equation}
      F_{iJ} = \delta_{iJ} + \frac{\partial u_i}{\partial X_J}
\end{equation}
with $u_i$ the displacement field and $\delta$ the Kronecker delta.

The strain calculator supports both small and large deformation kinematics, selected with the
[`large_kinematics`](#large_kinematics) option.  Under small deformation theory the displacement
gradients are assumed small enough that the reference and current configurations coincide, the
squared displacement-gradient terms are dropped, and only the strain tensors remain meaningful (see
[Kinematic Measures](#measures)).  Under large deformation theory the full nonlinear map is retained.

The gradients above are formed for three coordinate systems: [3D Cartesian](GradientOperator.md#3D_cartesian),
[2D axisymmetric cylindrical](GradientOperator.md#2D_axisymmetric_cylindrical), and
[1D centrosymmetric spherical](GradientOperator.md#1D_centrosymmetric_spherical).  The
[implementation](#implementation) section lists the strain calculator for each.

## Kinematic Measures id=measures

From the deformation gradient the strain calculator derives the incremental and accumulated measures
listed in [kinematics].  Throughout, a superscript $(n)$ denotes a quantity from the previous step;
a quantity written without a step superscript is evaluated at the current step.  The increments are
built from the inverse incremental deformation gradient
\begin{equation}
   f^{-1}_{ij} = F^{(n)}_{iK} \, F^{-1}_{Kj}
\end{equation}
which relates the previous configuration, $F^{(n)}$, to the current configuration, $F$.  The
calculator converts $f^{-1}$ into the increment in the spatial
velocity gradient, $\Delta l_{ij}$, and splits it into the symmetric strain increment
$\Delta d_{ij} = \tfrac{1}{2}(\Delta l_{ij} + \Delta l_{ji})$ and the skew vorticity increment
$\Delta w_{ij} = \tfrac{1}{2}(\Delta l_{ij} - \Delta l_{ji})$.

!table id=kinematics caption=Standard kinematic quantities provided by `ComputeLagrangianStrain`.
| Quantity name                            | Definition, `large_kinematics=true`                                           | Definition, `large_kinematics=false`                       |
|------------------------------------------|-------------------------------------------------------------------------------|------------------------------------------------------------|
| Deformation gradient                     | $F_{iJ} = \delta_{iJ} + \frac{\partial u_i}{\partial X_J}$                    | $F_{ij} = \delta_{ij} + \frac{\partial u_i}{\partial x_j}$ |
| Inverse deformation gradient             | $F^{-1}_{Ji}$                                                                 | $\delta_{ji}$                                              |
| Inverse incremental deformation gradient | $f^{-1}_{ij} = F^{(n)}_{iK} F^{-1}_{Kj}$                              | $f_{ij} = \delta_{ij}$                                     |
| Volume change                            | $J = \det F$                                                                  | $J = 1$                                                    |
| Total strain increment                   | $\Delta d_{ij} = \frac{1}{2} \left( \Delta l_{ij} + \Delta l_{ji} \right)$    | Same                                                       |
| Mechanical strain increment              | $\Delta \varepsilon_{ij} = \Delta d_{ij} - \Delta \varepsilon_{ij}^{(\text{eigen})}$ | Same                                                       |
| Total strain                             | $d_{ij} = d_{ij}^{(n)} + \Delta d_{ij}$                             | Same                                                       |
| Mechanical strain                        | $\varepsilon_{ij} = \varepsilon_{ij}^{(n)} + \Delta \varepsilon_{ij}$       | Same                                                       |

Except for the deformation gradient itself, all of the large-deformation quantities used to map
between configurations degenerate to the identity when `large_kinematics = false`, leaving only the
strain tensors defined.  Constitutive models are free to use whichever of these measures is most
convenient; some stress base classes provide additional measures (for example the Green-Lagrange
strain $E_{IJ} = \tfrac{1}{2}(F_{kI}F_{kJ} - \delta_{IJ})$ supplied by the second Piola-Kirchhoff
base class).

### Kinematic Approximations

For large deformations the conversion of $f^{-1}$ into $\Delta l$ is not unique: it is a closed-form
approximation of $-\log f^{-1}$.  The `kinematic_approximation` parameter selects among a `linear`
default and three higher-accuracy options; the same choice also sets the vorticity increment consumed
by the objective stress rates.  See [Kinematic Approximations](KinematicApproximations.md) for the
formulas and guidance.

!alert note title=Incremental finite-strain accuracy
Because the finite-strain measures are integrated incrementally, they carry a small path-dependence
error: over a closed deformation cycle the integrated rate of deformation does not return exactly to
zero, leaving a spurious residual stress [!cite](belytschko2003).  The error grows with the strain
per step and is negligible for typical increments; at very large strains a hyperelastic model (which
defines stress directly from the deformation gradient) avoids it entirely.  The higher-accuracy
[kinematic approximations](KinematicApproximations.md) reduce the incremental error where it matters.

## Eigenstrains id=eigenstrains

An eigenstrain is a strain that does not arise directly from mechanical loading, such as thermal
expansion.  The strain calculator sums the eigenstrain increments contributed by eigenstrain
materials (for example [`ComputeThermalExpansionEigenstrain`](ComputeThermalExpansionEigenstrain.md))
and subtracts them from the total strain increment to form the mechanical strain increment,
\begin{equation}
   \Delta \varepsilon_{ij} = \Delta d_{ij} - \Delta \varepsilon_{ij}^{(\text{eigen})} ,
\end{equation}
so a constitutive model written against the mechanical strain automatically accounts for thermal and
other stress-free strains.  Add eigenstrains through the `eigenstrain_names` parameter of the strain
calculator or the [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) action.

!alert note
Eigenstrains are applied incrementally to the strain measures only.  They do not modify the
deformation gradient, its inverse, or the inverse incremental deformation gradient, nor any measure
derived from those tensors.

## Stabilization id=stabilization

Setting `stabilize_strain = true` applies an $\bar{F}$ modification that averages the
volumetric part of the deformation gradient over each element, preventing volumetric locking of
linear quad and hex elements in incompressible or nearly-incompressible problems.  Because the
modification is applied to the deformation gradient before the derived measures are formed, it
propagates through every kinematic quantity in [kinematics].  The `F_bar_mode` parameter selects
whether the volumetric average is taken of the total (default) or incremental deformation gradient.
See the [stabilization system](Stabilization.md) documentation for the theory and for guidance on
when to enable it.

## Homogenization id=homogenization

When the [homogenization system](Homogenization.md) is active, the strain calculator adds the extra
macroscopic gradient supplied by [`ComputeHomogenizedLagrangianStrain`](ComputeHomogenizedLagrangianStrain.md)
to the displacement-based deformation gradient before forming any of the derived measures.  Unlike
the eigenstrain modification, the homogenization gradient therefore affects constitutive models
written against *any* kinematic tensor.

## The `large_kinematics` Option id=large_kinematics

The `large_kinematics` flag selects between large deformation theory (`true`) and small deformation
theory (`false`).  The strain calculator is the single source of truth for this choice: the stress
calculators and the kernels derive their kinematic theory from the strain calculator automatically,
so the flag only needs to be set in one place.

The recommended way to set it is through the
[SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) action, which configures
the strain calculator and kernels together from a single `strain = SMALL` or `strain = FINITE`
option.

!alert note
Setting `large_kinematics` directly on a stress calculator or kernel is deprecated: the value is
derived from the strain calculator, and an inconsistent explicit setting is reported as an error
rather than silently producing wrong results.

## Implementation id=implementation

The kinematics are computed by [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md) for Cartesian
coordinates, with variants for the other supported coordinate systems and for weak plane stress:

| Object | Use |
|--------|-----|
| [`ComputeLagrangianStrain`](ComputeLagrangianStrain.md) | 3D and 2D Cartesian problems |
| [`ComputeLagrangianStrainAxisymmetricCylindrical`](ComputeLagrangianStrainAxisymmetricCylindrical.md) | 2D axisymmetric cylindrical problems |
| [`ComputeLagrangianStrainCentrosymmetricSpherical`](ComputeLagrangianStrainCentrosymmetricSpherical.md) | 1D centrosymmetric spherical problems |
| [`ComputeLagrangianWPSStrain`](ComputeLagrangianWPSStrain.md) | weak plane stress problems |

In most cases the strain calculator is added automatically by the
[SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) action.  It can also be
added directly; the following shows the strain calculator (and the rest of the material setup) for a
large-deformation problem:

!listing modules/solid_mechanics/test/tests/lagrangian/cartesian/updated/patch/large_patch.i
         block=Materials

!bibtex bibliography
