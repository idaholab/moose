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
  # Use as many different time sequence steppers as we could to test the compositionDT
  [TimeSteppers]
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
      type = CSVTimeSequenceStepper
      file_name = timesequence.csv
      column_name = time
    []

    [Timesequence3]
      type = ExodusTimeSequenceStepper
      mesh = timesequence.e
    []
  []
[]

[Postprocessors]
  [timestep]
    type = TimePostprocessor
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  file_base='multiple_timesequences'
[]
