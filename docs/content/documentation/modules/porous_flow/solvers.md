#Preconditioners and linear solvers

MOOSE allows users to utilise the full power of the PETSc preconditioners and
linear solvers.  The following choices have been found to be effective for
various types of PorousFlow simulations.

- `pc_type = bjacobi, ksp_type = bcgs`
- `pc_type = bjacobi, ksp_type = gmres`
- `pc_type = asm, pc_asm_overlap = 2, sub_pc_type = lu, sub_pc_factor_shift_type = NONZERO, ksp_type = gmres`
- `pc_type = asm, pc_asm_overlap = 2, sub_pc_type = lu, sub_pc_factor_shift_type = NONZERO, ksp_type = gmres` along with the following options `-ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt`

These options can be set in the `Preconditioning` block in the input file

!listing modules/porous_flow/test/tests/newton_cooling/nc04.i block=Preconditioning caption=Example of using solver options
