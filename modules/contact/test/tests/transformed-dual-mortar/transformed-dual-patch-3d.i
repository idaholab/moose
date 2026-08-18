# 3D uniform-compression mortar-contact patch test on quadratic secondary faces,
# solved with an SMP preconditioner and a direct (LU) linear solve. The shared
# mesh, variables, physics, contact, and postprocessors live in
# transformed-dual-3d-base.i; this input adds only the preconditioner and
# executioner. The tests spec drives both element families (HEX20 / TET10) from
# this input via cli_args (Mesh/gen/elem_type).

!include transformed-dual-3d-base.i

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type '
                        '-pc_factor_shift_amount'
  petsc_options_value = 'lu    superlu_dist nonzero 1e-10'

  line_search = 'none'

  dt = 0.5
  dtmin = 0.1
  end_time = 1.0

  l_max_its = 20

  nl_max_its = 20
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  snesmf_reuse_base = false
[]
