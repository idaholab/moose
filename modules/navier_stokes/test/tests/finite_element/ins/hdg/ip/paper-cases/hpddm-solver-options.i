[Preconditioning]
  [FSP]
    type = SCFSP
    topsplit = 'up'
    [up]
      splitting = 'u p'
      splitting_type = schur
      petsc_options = '-ksp_monitor'
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition -ksp_gmres_restart -ksp_type -ksp_pc_side -ksp_rtol -ksp_max_it -ksp_atol'
      petsc_options_value = 'lower                           self                              300                fgmres    right        1e-4      300         1e-9'
    []
    [u]
      vars = 'vel_bar_x vel_bar_y'
      petsc_options = '-ksp_converged_reason -pc_hpddm_define_subdomains -pc_hpddm_levels_1_st_share_sub_ksp'
      petsc_options_iname = '-ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -ksp_max_it -ksp_atol -ksp_norm_type -pc_type -pc_hpddm_harmonic_overlap -pc_hpddm_levels_1_sub_pc_type -pc_hpddm_levels_1_sub_pc_factor_mat_solver_type -pc_hpddm_levels_1_svd_nsv -pc_hpddm_levels_1_svd_type -pc_hpddm_levels_1_svd_max_it -pc_hpddm_levels_1_svd_tol -pc_hpddm_levels_1_svd_threshold_relative -pc_hpddm_levels_1_bv_orthog_type -pc_hpddm_coarse_p -pc_hpddm_coarse_pc_type -pc_hpddm_coarse_pc_factor_mat_solver_type -pc_hpddm_coarse_pc_factor_shift_type -pc_hpddm_coarse_correction'
      petsc_options_value = 'gmres     1e-2      300                right        300         1e-8      unpreconditioned hpddm 3 lu mumps 150 lanczos 50 1e-3 1e-2 mgs 1 lu mumps NONZERO deflated'
    []
    [p]
      vars = 'pressure_bar'
      petsc_options = '-ksp_converged_reason'
      petsc_options_iname = '-pc_type -pc_jacobi_type -ksp_type -ksp_rtol -ksp_gmres_restart -ksp_pc_side -ksp_max_it -ksp_atol -ksp_norm_type'
      petsc_options_value = 'jacobi   rowsum          fgmres    1e-2      300                right        300         1e-8      unpreconditioned'
    []
  []
[]
