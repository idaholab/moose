# Tensor Mechanics Master Action System

!syntax description /Modules/TensorMechanics/Master/TensorMechanicsAction

The TensorMechanics Master Action is a convenience object that simplifies part of the
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

The Tensor Mechanics `Master` Action is used to construct the kernels, displacement variables, and strain materials in a consistent manner as required for a continuum mechanics simulation simulation. Optionally it generates aux variables and auxkernels to aid in the output of tensor components and scalar quantities.

### For the `StressDivergenceTensors` Kernels

!table id=tmMaster_action_table_sdt caption=Correspondence Among Action Functionality and MooseObjects for the Tensor Mechanics `Master` Action, current kernel system
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculate stress divergence equilibrium for the given coordinate system | [StressDivergenceTensors](/StressDivergenceTensors.md) and optionally [WeakPlaneStress](/WeakPlaneStress.md) or [StressDivergenceRZTensors](/StressDivergenceRZTensors.md) or [StressDivergenceRSphericalTensors](/StressDivergenceRSphericalTensors.md) | `displacements` : a string of the displacement field variables |
| Add the displacement variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean |
| Calculation of strain for the given coordinate system | [ComputeFiniteStrain](/ComputeFiniteStrain.md) or [ComputePlaneFiniteStrain](/ComputePlaneFiniteStrain.md) or [ComputeAxisymmetric1DFiniteStrain](/ComputeAxisymmetric1DFiniteStrain.md) or [ComputeAxisymmetricRZFiniteStrain](/ComputeAxisymmetricRZFiniteStrain.md) | `strain`: MooseEnum to select finite or strain formulations |
|   | [ComputeSmallStrain](/ComputeSmallStrain.md) or [ComputePlaneSmallStrain](/ComputePlaneSmallStrain.md) or [ComputeAxisymmetric1DSmallStrain](/ComputeAxisymmetric1DSmallStrain.md) or [ComputeAxisymmetricRZSmallStrain](/ComputeAxisymmetricRZSmallStrain.md) |   |
|   | [ComputeIncrementalSmallStrain](/ComputeIncrementalSmallStrain.md) or [ComputePlaneIncrementalStrain](/ComputePlaneIncrementalStrain.md) or [ComputeAxisymmetric1DIncrementalStrain](/ComputeAxisymmetric1DIncrementalStrain.md) or [ComputeAxisymmetricRZIncrementalStrain](/ComputeAxisymmetricRZIncrementalStrain.md) | `incremental` : boolean for using a incremental strain formulation |
| Add AuxVariables and AuxKernels for various tensor component and quantity outputs | Material Properties as well as [AuxVariables](/AuxVariables/index.md) and [RankTwoAux](/RankTwoAux.md) or [RankTwoScalarAux](/RankTwoScalarAux.md) or [RankFourAux](/RankFourAux.md) | `generate_output`: a string of the quantities to add |
| Add Material Properties for various tensor component and quantity outputs |  | `generate_output`: a string of the quantities to add |
| Add the optional global strain contribution to the strain calculation | Couples the [GlobalStrain](/GlobalStrain/index.md) system | `global_strain`: name of the material property that computes the global strain tensor |

Note that there are many variations for the calculation of the stress divergence and the strain measure. Review the theoretical introduction for the [Stress Divergence](tensor_mechanics/StressDivergence.md) and the [Strain Formulations](tensor_mechanics/Strains.md) for more information.

### For the New Lagrangian Kernel system

!table id=tmMaster_action_table_lc caption=Correspondence Among Action Functionality and MooseObjects for the Tensor Mechanics `Master` Action, new kernel system
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Calculate stress divergence equilibrium for the given coordinate system | [TotalLagrangianStressDivergence](/TotalLagrangianStressDivergence.md) or [UpdatedLagrangianStressDivergence](/UpdatedLagrangianStressDivergence.md) | `displacements` : a string of the displacement field variables, `formulation` : a MooseEnum controlling if the `UPDATED` or `TOTAL` Lagrangian formulation is used |
| Add the displacement variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean |
| Calculation of strain for the given coordinate system | [ComputeLagrangianStrain](/ComputeLagrangianStrain.md) | `strain`: MooseEnum to select finite or small kinematic formulations |
| Add AuxVariables and AuxKernels for various tensor component and quantity outputs | Material Properties as well as [AuxVariables](/AuxVariables/index.md) and [RankTwoAux](/RankTwoAux.md) or [RankTwoScalarAux](/RankTwoScalarAux.md) or [RankFourAux](/RankFourAux.md) | `generate_output`: a string of the quantities to add |
| Add Material Properties for various tensor component and quantity outputs |  | `generate_output`: a string of the quantities to add |
| Add the optional homogenization constraints | Adds all objects required to impose the [homogenization constraints](Homogenization.md) | `constraint_types` : MooseEnum controlling whether `strain` or `stress` constraints and imposed, `targets` : Functions providing the time-dependent targets  |

!alert warning If the using the TensorMechanicsAction with
`automatic_eigenstrain_names = true`, the eigenstrain_names will be populated
under restrictive conditions for classes such as
[CompositeEigenstrain](CompositeEigenstrain.md),
[ComputeReducedOrderEigenstrain](ComputeReducedOrderEigenstrain.md), and
[RankTwoTensorMaterialADConverter](MaterialADConverter.md).  The input components for
these classes are not included in the
[!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/eigenstrain_names) passed to the
`TensorMechanicsAction`.  Set the `automatic_eigenstrain_names = false` and
populate this list manually if these components need to be included.

## Example Input File Syntax

### New Kernel System

The following example sets up the new *Lagrangian* kernel system with a total Lagrangian formulation for a
large displacement kinematics problem.

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/total/action/action_L.i block=Modules/TensorMechanics/Master

### New Kernel System, with Homogenization Constraints

The following uses the action to setup homogenization constraints in a problem using the new kernel system.

!listing modules/tensor_mechanics/test/tests/lagrangian/cartesian/total/homogenization/action/action_3d.i block=Modules/TensorMechanics/Master

### Subblocks

The subblocks of the Master action are what triggers MOOSE objects to be built.
If none of the mechanics is subdomain restricted a single subblock can be used

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Modules/TensorMechanics/Master

if different mechanics models are needed, multiple subblocks with subdomain restrictions
can be used.

!listing modules/tensor_mechanics/test/tests/action/two_block_new.i block=Modules/TensorMechanics/Master

Parameters supplied at the `[Modules/TensorMechanics/Master]` level act as
defaults for the Master action subblocks.

!syntax parameters /Modules/TensorMechanics/Master/TensorMechanicsAction

## Associated Actions

!syntax list /Modules/TensorMechanics/Master objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/Master objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/Master objects=False actions=True subsystems=False
