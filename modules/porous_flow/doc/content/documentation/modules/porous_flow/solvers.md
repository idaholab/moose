# Preconditioners and linear solvers

MOOSE allows users to utilise the full power of the PETSc preconditioners and
linear solvers.  The following choices have been found to be effective for
various types of PorousFlow simulations.

- `pc_type = lu, pc_factor_mat_solver_package = mumps` (the most robust choice)
- `pc_type = bjacobi, ksp_type = bcgs`
- `pc_type = bjacobi, ksp_type = gmres`
- `pc_type = asm, pc_asm_overlap = 2, sub_pc_type = lu, sub_pc_factor_shift_type = NONZERO, ksp_type = gmres`
- `pc_type = asm, pc_asm_overlap = 2, sub_pc_type = lu, sub_pc_factor_shift_type = NONZERO, ksp_type = gmres` along with the following options `-ksp_diagonal_scale -ksp_diagonal_scale_fix -ksp_gmres_modifiedgramschmidt`

These options can be set in the `Preconditioning` block in the input file

!listing modules/porous_flow/test/tests/newton_cooling/nc04.i block=Preconditioning caption=Example of using solver options

The "mumps" method is usually superior to the "asm + LU" method, but isn't always installed on all computer systems.  Increasing the `pc_asm_overlap` improves the strength of the preconditioner, but uses a huge amount of memory.  The `NONZERO` shifting helps reduce problems when the diagonal of the Jacobian contains zeroes.

