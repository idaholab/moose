# Solid Mechanics QuasiStatic Physics

!syntax description /Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics

The [SolidMechanics/QuasiStatic](/Physics/SolidMechanics/QuasiStatic/index.md) action assembles a
consistent set of MOOSE objects for a solid mechanics problem.  User-facing documentation,
parameters, and examples are on the
[system page](/Physics/SolidMechanics/QuasiStatic/index.md); this page records the objects the action
constructs and the compatibility-mode wiring for reference.

## Constructed Objects id=constructed

The action applies to both the legacy [StressDivergenceTensors](/StressDivergenceTensors.md) kernels
and the [Lagrangian](/TotalLagrangianStressDivergence.md) kernels; some steps apply to only one
system:

- Add the stress-divergence kernels for the current coordinate system -- both systems, Cartesian only
  for the Lagrangian kernels
- Add the [WeakPlaneStress](/WeakPlaneStress.md) kernel -- only the `StressDivergenceTensors` system
- Add the strain calculation material for the chosen strain model -- both systems
- Set `use_displaced_mesh` correctly -- both systems
- Optional: set up the displacement variables at the correct order -- both systems
- Optional: add AuxVariables and AuxKernels for tensor-component and scalar outputs -- both systems
- Optional: set up out-of-plane stress/strain consistently -- only the `StressDivergenceTensors` system
- Optional: extract eigenstrain names from materials and apply them to the proper blocks -- both systems
- Optional: set up cell-average homogenization constraints -- only the Lagrangian kernels

### For the `StressDivergenceTensors` Kernels

!table id=action_table_sdt caption=Action functionality and constructed objects, legacy `StressDivergenceTensors` kernel system.
| Functionality     | Constructed Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculate stress divergence equilibrium for the given coordinate system | [StressDivergenceTensors](/StressDivergenceTensors.md) and optionally [WeakPlaneStress](/WeakPlaneStress.md) or [StressDivergenceRZTensors](/StressDivergenceRZTensors.md) or [StressDivergenceRSphericalTensors](/StressDivergenceRSphericalTensors.md) | `displacements` : a string of the displacement field variables |
| Add the displacement variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean |
| Calculation of strain for the given coordinate system | [ComputeFiniteStrain](/ComputeFiniteStrain.md) or [ComputePlaneFiniteStrain](/ComputePlaneFiniteStrain.md) or [ComputeAxisymmetric1DFiniteStrain](/ComputeAxisymmetric1DFiniteStrain.md) or [ComputeAxisymmetricRZFiniteStrain](/ComputeAxisymmetricRZFiniteStrain.md) | `strain`: MooseEnum to select finite or strain formulations |
|   | [ComputeSmallStrain](/ComputeSmallStrain.md) or [ComputePlaneSmallStrain](/ComputePlaneSmallStrain.md) or [ComputeAxisymmetric1DSmallStrain](/ComputeAxisymmetric1DSmallStrain.md) or [ComputeAxisymmetricRZSmallStrain](/ComputeAxisymmetricRZSmallStrain.md) |   |
|   | [ComputeIncrementalStrain](/ComputeIncrementalStrain.md) or [ComputePlaneIncrementalStrain](/ComputePlaneIncrementalStrain.md) or [ComputeAxisymmetric1DIncrementalStrain](/ComputeAxisymmetric1DIncrementalStrain.md) or [ComputeAxisymmetricRZIncrementalStrain](/ComputeAxisymmetricRZIncrementalStrain.md) | `incremental` : boolean for using a incremental strain formulation |
| Add AuxVariables and AuxKernels for various tensor component and quantity outputs | Material Properties as well as [AuxVariables](/AuxVariables/index.md) and [RankTwoAux](/RankTwoAux.md) or [RankTwoScalarAux](/RankTwoScalarAux.md) or [RankFourAux](/RankFourAux.md) | `generate_output`: a string of the quantities to add |
| Add Material Properties for various tensor component and quantity outputs |  | `generate_output`: a string of the quantities to add |
| Add the optional global strain contribution to the strain calculation | Couples the [GlobalStrain](/SolidMechanics/GlobalStrain/index.md) system | `global_strain`: name of the material property that computes the global strain tensor |

### For the Lagrangian Kernel System

!table id=action_table_lc caption=Action functionality and constructed objects, Lagrangian kernel system.
| Functionality     | Constructed Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculate stress divergence equilibrium for the given coordinate system | [TotalLagrangianStressDivergence](/TotalLagrangianStressDivergence.md) or [UpdatedLagrangianStressDivergence](/UpdatedLagrangianStressDivergence.md) | `displacements` : a string of the displacement field variables, `formulation` : a MooseEnum controlling if the `UPDATED` or `TOTAL` Lagrangian formulation is used |
| Add the displacement variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean |
| Calculation of strain for the given coordinate system | [ComputeLagrangianStrain](/ComputeLagrangianStrain.md) | `strain`: MooseEnum to select finite or small kinematic formulations |
| Add AuxVariables and AuxKernels for various tensor component and quantity outputs | Material Properties as well as [AuxVariables](/AuxVariables/index.md) and [RankTwoAux](/RankTwoAux.md) or [RankTwoScalarAux](/RankTwoScalarAux.md) or [RankFourAux](/RankFourAux.md) | `generate_output`: a string of the quantities to add |
| Add Material Properties for various tensor component and quantity outputs |  | `generate_output`: a string of the quantities to add |
| Add the optional homogenization constraints | Adds all objects required to impose the [homogenization constraints](Homogenization.md) | `constraint_types` : MooseEnum controlling whether `strain` or `stress` constraints and imposed, `targets` : Functions providing the time-dependent targets  |

## Compatibility Mode Wiring id=compat-wiring

Setting `compatibility_mode = true` configures the
[Lagrangian kernel system](BalanceOfLinearMomentum.md) to reproduce the legacy
[`StressDivergenceTensors`](/StressDivergenceTensors.md) + [`ComputeFiniteStrain`](/ComputeFiniteStrain.md)
+ `ComputeStressBase`-style pipeline bit-for-bit.  [compat_table] summarizes the wiring the action
sets up automatically.

!table id=compat_table caption=Mapping from OLD-style parameters to NEW-system settings under `compatibility_mode = true`.
| OLD-style parameter or quantity                    | NEW-system equivalent set automatically                                              |
|----------------------------------------------------|--------------------------------------------------------------------------------------|
| (implied)                                          | `new_system = true`, `formulation = TOTAL`                                           |
| `decomposition_method = TaylorExpansion`           | `kinematic_approximation = rashid_approximate` on the strain calculator              |
| `decomposition_method = EigenSolution`             | `kinematic_approximation = rashid_eigen` on the strain calculator                    |
| `decomposition_method = HughesWinget`              | Rejected at construction (no NEW equivalent)                                         |
| `volumetric_locking_correction = true` (FINITE)    | `F_bar_mode = incremental` on the strain calc *and* the kernel                       |
| user's `ComputeStressBase`-style stress material   | Auto-wrapped with [`ComputeLagrangianWrappedStress`](/ComputeLagrangianWrappedStress.md) using `objective_rate = rashid` and `rotate_old_stress = true` |
| (implied for the wrapped material's FSR)           | `publish_rotation_increment = true` on the strain calc (when `strain = FINITE`)      |
| `generate_output = stress_*`                       | Redirected to read from `cauchy_stress_*`                                            |
| `generate_output = mechanical_strain_*`            | Redirected to read from `rotated_mechanical_strain_*`                                |

The user's stress material remains the OLD-style `ComputeStressBase` descendant (e.g.
[`ComputeMultiPlasticityStress`](/ComputeMultiPlasticityStress.md)); the action adds the wrapper
material around it.  See [`ComputeLagrangianObjectiveStress`](/ComputeLagrangianObjectiveStress.md)
for the Rashid rate and the [kinematic approximations](/solid_mechanics/KinematicApproximations.md)
page for the `rashid_approximate` / `rashid_eigen` increments.

The action enforces several constraints in compatibility mode:

- `formulation = UPDATED` is rejected; the OLD-system equivalence is only defined for the total
  Lagrangian formulation.
- `decomposition_method = HughesWinget` is rejected at construction because the Lagrangian kernel
  system has no `HughesWinget` counterpart.
- `strain = SMALL` leaves `F_bar_mode` at its default `total` value; the strain calculator rejects
  `incremental` for small kinematics.
