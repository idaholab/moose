[Preconditioning]
  [FSP]
    type = SCFSP
    topsplit = 'up'
    [up]
      splitting = 'u p'
      splitting_type = schur
      petsc_options = '-ksp_converged_reason -ksp_monitor'
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_type -ksp_pc_side -ksp_rtol -ksp_max_it -ksp_atol'
      petsc_options_value = 'lower                           self                              300                fgmres    right        1e-4      300         1e-9'
    []
    [u]
      vars = 'vel_bar_x vel_bar_y'
      petsc_options = '-ksp_converged_reason'
      petsc_options_iname = '-pc_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -pc_factor_mat_solver_type -ksp_max_it -ksp_atol -ksp_norm_type'
      petsc_options_value = 'ilu      gmres     1e-2      300                right        strumpack                  300         1e-8      unpreconditioned'
    []
    [p]
      vars = 'pressure_bar'
      petsc_options = '-ksp_converged_reason'
      petsc_options_iname = '-pc_type -pc_jacobi_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -ksp_max_it -ksp_atol -ksp_norm_type'
      petsc_options_value = 'jacobi   diagonal        fgmres    1e-2      300                right        300         1e-8      unpreconditioned'
    []
  []
[]
