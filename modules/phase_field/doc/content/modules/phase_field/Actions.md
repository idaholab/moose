# Custom Phase Field Actions

To simplify the formation of input files that use standard Allen-Cahn and Cahn-Hilliard equations,
custom actions have been created that automatically create conserved and nonconserved phase field
field variables and all the corresponding kernels. Additional kernels can still be added using
standard MOOSE syntax.

The actions are in the phase field block, under the modules block. Nonconserved variables are created
using [NonconservedAction](/NonconservedAction.md). For an example, see

!listing modules/phase_field/test/tests/actions/Nonconserved_1var.i start=Modules end=ICs

Conserved variables are created using [ConservedAction](/ConservedAction.md). For an example, See

!listing modules/phase_field/test/tests/actions/conserved_split_1var.i start=Modules end=ICs

and

!listing modules/phase_field/test/tests/actions/conserved_direct_1var.i start=Modules end=ICs



## See also

- [Phase Field FAQ](FAQ) - Frequently asked questions for MOOSE phase-field models.
