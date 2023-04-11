[Mesh]
  file = restart_test_cp/0002_mesh.cpr
[]

[Problem]
  restart_file_base = restart_test_cp/0002
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  end_time = 1.2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  final_time_stepper = 'CompositionDT'
  # Pluggable TimeStepper System
  [TimeStepper]
    [ConstDT1]
      type = ConstantDT
      dt = 0.2
    []

    [ConstDT2]
      type = ConstantDT
      dt = 0.3
    []

    [LogConstDT]
      type = LogConstantDT
      log_dt = 0.2
      first_dt = 0.1
    []

    [Timesequence]
      type = TimeSequenceStepper
      time_sequence  = '0  0.5 0.8 1 1.2'
    []

    [CompositionDT]
      type = CompositionDT
      input_timesteppers = 'ConstDT1 LogConstDT'
      base_timestepper = 'ConstDT2'
      composition_type = 'max'
      times_to_hit_timestepper = 'Timesequence'
    []
  []
[]

[Outputs]
  exodus=true
[]
