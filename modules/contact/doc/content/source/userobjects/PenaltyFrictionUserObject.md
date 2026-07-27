# PenaltyFrictionUserObject

## Description

The `PenaltyFrictionUserObject` object computes tangential pressures due
to Coulomb friction by looping over mortar segments. The accumulated tangential
slip is penalized by a `penalty_friction` factor. As the slip accumulates, the
frictional pressure(s) saturates at the Coulomb limit. Note that a high enough
value of the `penalty_friction` value is required to obtain realistic frictional
pressure; otherwise, the tangential model will underpredict frictional forces. That is,
higher values of the `penalty_friction` parameter will more accurately predict the stick
and slip Coulomb behavior at the expense of worsening the convergence behavior. The
`slip_tolerance` parameter can be chosen to limit the amount of relative slip
at nodes that are not slipping (at the Coulomb limit).

The tangential traction (or tractions for a three-dimensional problem) generated
by this object contains Jacobian information as a consequence of using automatic
differentiation objects. These tractions are directly applied on the contacting
surfaces by [TangentialMortarMechanicalContact](/TangentialMortarMechanicalContact.md).
That is, the tangential contact pressure values computed by this object serve a purpose
analogous to the Lagrange multipliers when exact enforcement is used
(see [ComputeFrictionalForceLMMechanicalContact](/ComputeFrictionalForceLMMechanicalContact.md)).
This object is set up automatically when using the [contact action](/ContactAction.md).

This object is built from within the contact action when the formulation `MORTAR_PENALTY` and
the model `COULOMB` is selected. The action sets the `use_physical_gap` parameter to true,
which allows the user to select a normal contact penalty parameter on the order of the
stiffness of the materials coming into contact. Also, to ensure solution stability, dual bases
(i.e. `use_dual = true`) are employed by default to interpolate the contact traction
when using the contact action.

An augmented Lagrange (AL) approach can be used to enforce the contact constraints to a user-prescribed
tolerance. That tolerance can be the normal gap distance (distance to exact enforcement if in contact) or
the relative slip tolerance for slipping nodes. The AL approach solves the original MOOSE problem,
in which contact is enforced using a pure penalty approach, taking the necessary nonlinear iterations
and updates "fixed" Lagrange multipliers in an outer loop. This
process repeats until the contact-related tolerances are met. The "fixed" Lagrange multipliers represent
accumulated normal and tangential tractions over the AL iterations (see [!citep](wriggers2006computational)).
Usage of AL with this mortar constraint allows for, simultaneously 1. Having a contact formulation that, upon AL convergence, will yield results equivalent to Lagrange multiplier-enforced dual mortar,
2. Enforcement of the contact constraint to a user-prescribed tolerance (analogous to Lagrange multiplier
enforcement, see [ComputeWeightedGapLMMechanicalContact](/ComputeWeightedGapLMMechanicalContact.md)), and 3. Keeps
the system's condition number as if mechanical contact is not present; as long as the initial penalty coefficients are
selected on the order of the material stiffnesses.

!alert tip title=Augmented Lagrange with mortar contact
For the reasons stated above, it is recommended to use an AL approach with the mortar penalty objects.

Example of usage in the contact action:

!listing modules/contact/test/tests/pdass_problems/cylinder_friction_penalty_frictional_al_action.i block=Contact

which can be combined with an augmented Lagrange problem:

!listing modules/contact/test/tests/pdass_problems/cylinder_friction_penalty_frictional_al_action.i block=Problem

See below an example of solver options with iterative preconditioners (e.g. algebraic multigrid) in a mortar
mechanical contact problem. This solver selection can enable the simulation of very large contact mechanics problems with the more memory efficient iterative procedures:

!listing modules/contact/test/tests/pdass_problems/cylinder_friction_penalty_frictional_al_action_amg.i block=Executioner


!syntax parameters /UserObjects/PenaltyFrictionUserObject

!syntax inputs /UserObjects/PenaltyFrictionUserObject

!syntax children /UserObjects/PenaltyFrictionUserObject
