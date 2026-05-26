# Solid Mechanics QuasiStatic Physics System

!syntax description /Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics

The Solid Mechanics Physics Action is a convenience [Action](Action.md) that simplifies part of the
mechanics system setup.

It applies to both the current kernel system based on the [StressDivergenceTensors](/StressDivergenceTensors.md) kernels
and to the new kernel system based on the
[TotalLagrangianStressDivergence](/TotalLagrangianStressDivergence.md) and [UpdatedLagrangianStressDivergence](/UpdatedLagrangianStressDivergence.md) kernels.
Some options only apply to one or the other system, as outlined below.

It performs

- Add StressDivergence Kernels (for the current coordinate system) -- both systems, only Cartesian coordinates for the *Lagrangian* kernel system
- Add WeakPlaneStress Kernel (for weak enforcement of the plane stress condition) -- only the `StressDivergenceTensors` system
- Add Strain calculation material (for the chosen strain model) -- both systems
- Correctly set use of displaced mesh -- both systems
- Optional: Setup of displacement variables (with the correct order for the current mesh) -- both systems
- Optional: Add AuxVariables and AuxKernels for various tensor components and quantity outputs -- both systems
- Optional: Set up out-of-plane stress/strain consistently -- only the `StressDivergenceTensors` system
- Optional: Automatic extraction of eigenstrain names from materials and correct application to proper blocks -- both systems
- Optional: Setup cell-average homogenization constraints on the simulation -- only the new *Lagrangian* kernels

## Constructed MooseObjects

The Solid Mechanics QuasiStatic Physics Action is used to construct the kernels, displacement variables, and strain materials in a consistent manner as required for a continuum mechanics simulation simulation. Optionally it generates aux variables and auxkernels to aid in the output of tensor components and scalar quantities.

### For the `StressDivergenceTensors` Kernels

!table id=tmMaster_action_table_sdt caption=Correspondence Among Action Functionality and MooseObjects for the Solid Mechanics QuasiStatic Physics Action, current kernel system
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculate stress divergence equilibrium for the given coordinate system | [StressDivergenceTensors](/StressDivergenceTensors.md) and optionally [WeakPlaneStress](/WeakPlaneStress.md) or [StressDivergenceRZTensors](/StressDivergenceRZTensors.md) or [StressDivergenceRSphericalTensors](/StressDivergenceRSphericalTensors.md) | `displacements` : a string of the displacement field variables |
| Add the displacement variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean |
| Calculation of strain for the given coordinate system | [ComputeFiniteStrain](/ComputeFiniteStrain.md) or [ComputePlaneFiniteStrain](/ComputePlaneFiniteStrain.md) or [ComputeAxisymmetric1DFiniteStrain](/ComputeAxisymmetric1DFiniteStrain.md) or [ComputeAxisymmetricRZFiniteStrain](/ComputeAxisymmetricRZFiniteStrain.md) | `strain`: MooseEnum to select finite or strain formulations |
|   | [ComputeSmallStrain](/ComputeSmallStrain.md) or [ComputePlaneSmallStrain](/ComputePlaneSmallStrain.md) or [ComputeAxisymmetric1DSmallStrain](/ComputeAxisymmetric1DSmallStrain.md) or [ComputeAxisymmetricRZSmallStrain](/ComputeAxisymmetricRZSmallStrain.md) |   |
|   | [ComputeIncrementalStrain](/ComputeIncrementalStrain.md) or [ComputePlaneIncrementalStrain](/ComputePlaneIncrementalStrain.md) or [ComputeAxisymmetric1DIncrementalStrain](/ComputeAxisymmetric1DIncrementalStrain.md) or [ComputeAxisymmetricRZIncrementalStrain](/ComputeAxisymmetricRZIncrementalStrain.md) | `incremental` : boolean for using a incremental strain formulation |
| Add AuxVariables and AuxKernels for various tensor component and quantity outputs | Material Properties as well as [AuxVariables](/AuxVariables/index.md) and [RankTwoAux](/RankTwoAux.md) or [RankTwoScalarAux](/RankTwoScalarAux.md) or [RankFourAux](/RankFourAux.md) | `generate_output`: a string of the quantities to add |
| Add Material Properties for various tensor component and quantity outputs |  | `generate_output`: a string of the quantities to add |
| Add the optional global strain contribution to the strain calculation | Couples the [GlobalStrain](/SolidMechanics/GlobalStrain/index.md) system | `global_strain`: name of the material property that computes the global strain tensor |

Note that there are many variations for the calculation of the stress divergence and the strain measure. Review the theoretical introduction for the [Stress Divergence](solid_mechanics/StressDivergence.md) and the [Strain Formulations](solid_mechanics/Strains.md) for more information.

### For the New Lagrangian Kernel system

!table id=tmMaster_action_table_lc caption=Correspondence Among Action Functionality and MooseObjects for the Solid Mechanics QuasiStatic Physics Action, new kernel system
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculate stress divergence equilibrium for the given coordinate system | [TotalLagrangianStressDivergence](/TotalLagrangianStressDivergence.md) or [UpdatedLagrangianStressDivergence](/UpdatedLagrangianStressDivergence.md) | `displacements` : a string of the displacement field variables, `formulation` : a MooseEnum controlling if the `UPDATED` or `TOTAL` Lagrangian formulation is used |
| Add the displacement variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean |
| Calculation of strain for the given coordinate system | [ComputeLagrangianStrain](/ComputeLagrangianStrain.md) | `strain`: MooseEnum to select finite or small kinematic formulations |
| Add AuxVariables and AuxKernels for various tensor component and quantity outputs | Material Properties as well as [AuxVariables](/AuxVariables/index.md) and [RankTwoAux](/RankTwoAux.md) or [RankTwoScalarAux](/RankTwoScalarAux.md) or [RankFourAux](/RankFourAux.md) | `generate_output`: a string of the quantities to add |
| Add Material Properties for various tensor component and quantity outputs |  | `generate_output`: a string of the quantities to add |
| Add the optional homogenization constraints | Adds all objects required to impose the [homogenization constraints](Homogenization.md) | `constraint_types` : MooseEnum controlling whether `strain` or `stress` constraints and imposed, `targets` : Functions providing the time-dependent targets  |

## Compatibility Mode id=compatibility-mode

Setting `compatibility_mode = true` configures the
[Lagrangian (new) kernel system](LagrangianKernelTheory.md) to reproduce the
legacy [`StressDivergenceTensors`](/StressDivergenceTensors.md) +
[`ComputeFiniteStrain`](/ComputeFiniteStrain.md) +
[`ComputeStressBase`](Stresses.md)-style pipeline bit-for-bit.  Use it to port an
existing OLD-system input to the new kernels by flipping a single flag, without
rewriting the rest of the block.

When `compatibility_mode = true`, the action automatically wires up the
combination of strain, kernel, and material settings that reproduces the legacy
update.  [compat_table] summarizes the wiring.

!table id=compat_table caption=Mapping from OLD-style parameters to NEW-system settings under `compatibility_mode = true`
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

The user's stress material continues to be the OLD-style
[`ComputeStressBase`](Stresses.md) descendant (e.g.
[`ComputeMultiPlasticityStress`](/ComputeMultiPlasticityStress.md)); the action
adds the wrapper material around it.  See
[`ComputeLagrangianObjectiveStress`](/ComputeLagrangianObjectiveStress.md) for
the Rashid rate and [the kinematic approximations page](/solid_mechanics/KinematicApproximations.md)
for the `rashid_approximate` / `rashid_eigen` increments.

The action enforces several constraints in compatibility mode:

- `formulation = UPDATED` is rejected; the OLD-system equivalence is only
  defined for the total Lagrangian formulation.
- `decomposition_method = HughesWinget` is rejected at construction because the
  Lagrangian kernel system has no `HughesWinget` counterpart.
- `strain = SMALL` leaves `F_bar_mode` at its default `total` value; the strain
  calculator rejects `incremental` for small kinematics.

### Example Input

The following uses compatibility mode to drive an OLD-style finite-strain
plasticity model through the Lagrangian kernels:

!listing modules/solid_mechanics/test/tests/lagrangian/cartesian/total/cross_material/interoperability/new_system_action.i block=Physics/SolidMechanics/QuasiStatic

!alert warning If the using the `Physics/SolidMechanics/QuasiStatic` with
`automatic_eigenstrain_names = true`, the eigenstrain_names will be populated
under restrictive conditions for classes such as
[CompositeEigenstrain](CompositeEigenstrain.md),
[ComputeReducedOrderEigenstrain](ComputeReducedOrderEigenstrain.md), and
[RankTwoTensorMaterialADConverter](MaterialADConverter.md).  The input components for
these classes are not included in the
[!param](/Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics/eigenstrain_names) passed to the
`Physics/SolidMechanics/QuasiStatic` block.  Set the `automatic_eigenstrain_names = false` and
populate this list manually if these components need to be included.

## Example Input File Syntax

### New Kernel System

The following example sets up the new *Lagrangian* kernel system with a total Lagrangian formulation for a
large displacement kinematics problem.

!listing modules/solid_mechanics/test/tests/lagrangian/cartesian/total/action/action_L.i block=Physics/SolidMechanics/QuasiStatic

### New Kernel System, with Homogenization Constraints

The following uses the action to setup homogenization constraints in a problem using the new kernel system.

!listing modules/solid_mechanics/test/tests/lagrangian/cartesian/total/homogenization/action/action_3d.i block=Physics/SolidMechanics/QuasiStatic

### Subblocks

The subblocks of the QuasiStatic Physics Action are what triggers MOOSE objects to be built.
If none of the mechanics is subdomain restricted a single subblock can be used

!listing modules/solid_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Physics/SolidMechanics/QuasiStatic

if different mechanics models are needed, multiple subblocks with subdomain restrictions
can be used.

!listing modules/solid_mechanics/test/tests/action/two_block_new.i block=Physics/SolidMechanics/QuasiStatic

Parameters supplied at the `[Physics/SolidMechanics/QuasiStatic]` level act as
defaults for the QuasiStatic Solid Mechanics Physics subblocks.

!syntax parameters /Physics/SolidMechanics/QuasiStatic/QuasiStaticSolidMechanicsPhysics

## Associated Actions

!syntax list /Physics/SolidMechanics/QuasiStatic objects=True actions=False subsystems=False

!syntax list /Physics/SolidMechanics/QuasiStatic objects=False actions=False subsystems=True

!syntax list /Physics/SolidMechanics/QuasiStatic objects=False actions=True subsystems=False
