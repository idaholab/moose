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
  num_steps = 10
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  final_time_stepper = 'composition'
  # Pluggable TimeStepper System
  [TimeStepper]
    [ConstDT]
      type = ConstantDT
      dt = 0.2
    []

    [LogConstDT]
      type = LogConstantDT
      log_dt = 0.2
      first_dt = 0.1
    []

    [composition]
      type = CompositionDT
      inputs = 'ConstDT LogConstDT'
      composition_type = 'average'
    []
  []
[]

[Outputs]
  exodus = true
[]
