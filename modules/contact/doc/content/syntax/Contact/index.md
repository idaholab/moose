# Contact

## Description

The `Contact` block can be used to specify parameters related to mechanical contact enforcement in
MOOSE simulations. The [ContactAction](/actions/ContactAction.md) is associated with this
input block, and is the class that performs the associated model setup tasks. Use of the ContactAction
is not strictly required, but it greatly simplifies the setup of a simulation using contact enforcement.
A high-level description of the contact problem is provided [here](modules/contact/index.md).

This block can be used to specify mechanical normal and tangential contact using several possible
models for the physical behavior of the interaction:

- frictionless
- glued
- coulomb (frictional)

Contact enforcement using node/face primary/secondary algorithms is available using the following mathematical
formulations:

- kinematic
- penalty
- tangential penalty (kinematic normal constraint with penalty tangential constraint)
- augmented lagrange
- reduced active nonlinear function set (RANFS)

In addition, face/face contact using a mortar method can also be specified using this block.

## Constructed Objects

The primary task performed by this action is creating the Constraint classes that perform the contact enforcement.
The type of Constraint class(es) constructed depend on the formulation and physical interaction model specified
using the `formulation` and `model` parameters. [contact_action_constraint_table] shows the Constraint classes
that can be created for various types of contact enforcement.

!table id=contact_action_constraint_table caption=Constraint objects constructed by ContactAction
| Formulation        | Model   |  Constraint Object   |
|--------------------|--------------------|--------------------|
| kinematic          | all          | [MechanicalContactConstraint](/constraints/MechanicalContactConstraint.md) |
| penalty            | all          | [MechanicalContactConstraint](/constraints/MechanicalContactConstraint.md) |
| tangential_penalty | all          | [MechanicalContactConstraint](/constraints/MechanicalContactConstraint.md) |
| ranfs              | frictionless | [RANFSNormalMechanicalContact](/constraints/RANFSNormalMechanicalContact.md) |
| mortar             | all          | [MechanicalContactConstraint](/constraints/MechanicalContactConstraint.md) |
| mortar             | all          | [NormalMortarMechanicalContact](/constraints/NormalMortarMechanicalContact.md) [ComputeWeightedGapLMMechanicalContact.md] |
| mortar             | coulomb      | [TangentialMortarMechanicalContact](/constraints/TangentialMortarMechanicalContact.md) [ComputeFrictionalForceLMMechanicalContact.md] |

In addition to the Constraint class, several other objects are created, as shown in

!table id=contact_action_otherobj_table caption=Other objects constructed by ContactAction
| Constructed Object | Purpose |
|--------------------|--------------------|
| [ContactPressureAux](/auxkernels/ContactPressureAux.md) | Compute contact pressure and store in an AuxVariable |
| [Penetration](/auxkernels/PenetrationAux.md) | Compute contact penetration and store in an AuxVariable |
| [NodalArea](/userobjects/NodalArea.md) | Compute nodal area and store in an AuxVariable |

## Notes on Node/Face Contact Enforcement

The node/face contact enforcement is based on a primary/secondary algorithm, in
which contact is enforced at the nodes on the secondary surface, which cannot
penetrate faces on the primary surface. As with all such approaches, for the
best results, the primary surface should be the coarser of the two surfaces.

The contact enforcement system relies on MOOSE's geometric search system to
provide the candidate set of faces that can interact with a secondary node at a
given time. The set of candidate faces is controlled by the `patch_size`
parameter and the `patch_update_strategy` options in the
[Mesh](/mesh/MooseMesh.md) block. The patch size must be large enough to
accommodate the sliding that occurs during a time step. It is generally
recommended that the `patch_update_strategy=auto` be used.

The formulation parameter specifies the technique used to enforce contact. The
DEFAULT option uses a kinematic enforcement algorithm that transfers the
internal forces at secondary nodes to the corresponding primary face, and forces the
secondary node to be at a specific location on the primary face using a penalty
parameter. The converged solution with this approach results no penetration
between the surfaces. The PENALTY algorithm employs a penalty approach, where
the penetration between the surfaces is penalized, and the converged solution
has a small penetration, which is inversely proportional to the penalty
parameter.

Regardless of the formulation used, the robustness of the mechanical contact
algorithm is affected by the penalty parameter. If the parameter is too small,
there will be excessive penetrations with the penalty formulation, and
convergence will suffer with the kinematic formulation. If the parameter is too
large, the solver may struggle due to poor conditioning.

## `System` Parameter

The `system` parameter is deprecated and currently defaults to `Constraint`.

## Gap offset parameters

Gap offset can be provided to the current contact formulation enforced using the [MechanicalContactConstraint](/constraints/MechanicalContactConstraint.md). It can be either `secondary_gap_offset` (gap offset from secondary side) or `mapped_primary_gap_offset` (gap offset from primary side but mapped to secondary side). Use of these gap offset parameters treats the surfaces as if they were virtually extended (positive offset value) or narrowed (negative offset value) by the specified amount, so that the surfaces are treated as if they are closer or further away than they actually are. There is no deformation within the material in this gap offset region.

## Multiple contact pairs

Users may need to set up mechanical contact between multiple contact pairs. For that application, users can provide arrays of primary and secondary boundary names which will match consecutively to define mechanical contact pairs. The same contact-related input parameters will be applied to all contact pairs defined in the action input.

!alert note
The multiple contact pairs feature is not yet available for mortar contact.

!listing test/tests/multiple_contact_pairs/multiple_pairs.i block=Contact

## Example Input syntax id=example

Node/face frictionless contact:

!listing test/tests/sliding_block/sliding/frictionless_kinematic.i block=Contact

Node/face frictional contact:

!listing test/tests/sliding_block/sliding/frictional_02_penalty.i block=Contact

Normal (frictionless) mortar contact:

!listing test/tests/bouncing-block-contact/ping-ponging/mortar-no-ping-pong_weighted.i block=Contact

Normal and tangential (frictional) mortar contact:

!listing test/tests/3d-mortar-contact/frictional-mortar-3d-action.i block=Contact

Gap offset:

!listing test/tests/mechanical_constraint/frictionless_kinematic_gap_offsets.i block=Contact

!syntax parameters /Contact/ContactAction

!syntax inputs /Contact/ContactAction

!syntax children /Contact/ContactAction
