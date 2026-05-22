!include physics.i
!include off_diag_coupling.i

[Preconditioning]
  [fsp]
    type = FSP
    topsplit = 'uv'
    [uv]
      splitting = 'u v'
      splitting_type = SYMMETRIC_MULTIPLICATIVE
    []
    [u]
      petsc_options_iname = '-pc_type'
      petsc_options_value = 'hypre'
      vars = 'u'
    []
    [v]
      petsc_options_iname = '-pc_type'
      petsc_options_value = 'hypre'
      vars = 'v'
    []
  []
[]

[Executors]
  [steady]
    type = SteadyExecutor
    inner_executors = 'newton'
  []
  [newton]
    type = NewtonSNESExecutor
    nonlinear_system_names = 'nl0'
  []
[]

