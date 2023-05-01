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
  # Pluggable TimeStepper System
  [TimeSteppers]
    [ConstDT1]
      type = ConstantDT
      dt = 0.2
    []

    [ConstDT2]
      type = ConstantDT
      dt = 0.1
    []
  []
[]

[Controls]
  [c1]
    type = TimePeriod
    enable_objects = 'TimeStepper::ConstDT1'
    disable_objects = 'TimeStepper::ConstDT2'
    start_time = '0'
    end_time = '0.3'
   # reverse_on_false = true
  []
[]


[Outputs]
  exodus = true
  file_base='active_timesteppers'
[]
