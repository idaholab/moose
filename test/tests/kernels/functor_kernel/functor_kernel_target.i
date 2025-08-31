[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [PredefinedPorepressure]
    type = FunctorKernel
    variable = 'u'
    mode = 'TARGET'
    functor_on_rhs = false
    functor = target_u
  []
[]

[Functions]
  [target_u]
    type = ParsedFunction
    expression = 'max(1, 1 + t)'
  []
[]


[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  l_tol = 1e-15
  l_max_its = 50

  nl_abs_tol = 1e-16
  nl_rel_tol = 1e-16
  nl_max_its = 15

  start_time = 0.0
  end_time = 11
  dtmin = 1e-2
  [TimeSteppers]
    [TimeSequenceStepper1]
      type = TimeSequenceStepper
      time_sequence = '1 2 3 4 5 6 7 8 9 10 11'
    []
  []

[]

[Outputs]
  exodus = true
[]
