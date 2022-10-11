# PenaltyPeriodicSegmentalConstraint

The `PenaltyPeriodicSegmentalConstraint` a periodic boundary condition between a microscale and 
macroscale field. Coupling is made between a scalar macro-gradient variable and the concentration field within
the periodic domain. Only  * the macro to micro coupling terms are handled here. The micro-micro coupling terms
are handled using the [PenaltyEqualValueConstraint](/PenaltyEqualValueConstraint.md) applied to the same 
primary/secondary pair.

The applied macroscale conjugate gradient is applied as `kappa_aux` vector as an auxillary
scalar. The computed macroscale gradient `kappa` is equal to this value for isotropic-unitary
diffusivity. The volume integral of the gradient of the primary field will be equal to these
imposed values.

The microscale variable is specified using the `primary_variable` parameter. 
If the solution values to be matched are between different variables, the
`secondary_variable` parameter can also be supplied. The enforcement takes place in a penalty sense, 
which eliminates the need to supply Lagrange multipliers.

!listing test/tests/mortar/periodic_segmental_constraint/testperiodic.i block=Constraints

!syntax description /Constraints/PenaltyPeriodicSegmentalConstraint

!syntax parameters /Constraints/PenaltyPeriodicSegmentalConstraint

!syntax inputs /Constraints/PenaltyPeriodicSegmentalConstraint

!syntax children /Constraints/PenaltyPeriodicSegmentalConstraint

!bibtex bibliography
