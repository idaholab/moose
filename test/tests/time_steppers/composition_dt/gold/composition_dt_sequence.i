###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of the TimeStepper system.
#
# @Requirement F1.20
###########################################################


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
  end_time = 0.8
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
      dt = 0.1
    []

    [LogConstDT]
      type = LogConstantDT
      log_dt = 0.2
      first_dt = 0.1
    []

    [Timesequence1]
      type = TimeSequenceStepper
      time_sequence  = '0  0.25 0.3 0.5 0.8'
    []

    [Timesequence2]
      type = TimeSequenceStepper
      time_sequence  = '0  0.45 0.65'
    []

    [CompositionDT]
      type = CompositionDT
      maximum_step_from = 'ConstDT1 ConstDT2'
      base_timestepper = 'LogConstDT'
      times_to_hit_timestepper = 'Timesequence1 Timesequence2'
    []
  []
[]

[Outputs]
  exodus = true
[]
