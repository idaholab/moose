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

[Functions]
  [dts]
    type = PiecewiseLinear
    x = '0   0.85 2'
    y = '0.2 0.15  0.2'
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

  [TimeSteppers]
    [ConstDT1]
      type = ConstantDT
      dt = 0.2
    []

    [FunctionDT]
      type = FunctionDT
      function = dts
    []

    [LogConstDT]
      type = LogConstantDT
      log_dt = 0.2
      first_dt = 0.1
    []

    [SolutionAdapDT]
      type = SolutionTimeAdaptiveDT
      dt = 0.5
    []

    [IterationAdapDT]
      type = IterationAdaptiveDT
      dt = 0.5
    []

    [Timesequence]
      type = TimeSequenceStepper
      time_sequence  = '0  0.25 0.3 0.5 0.8'
    []

    [PPDT]
      type = PostprocessorDT
      postprocessor = PostDT
      dt = 0.1
    []
  []
[]

[Postprocessors]
  [timestep]
    type = TimePostprocessor
    execute_on = 'timestep_end'
  []

  [PostDT]
    type = ElementAverageValue
    variable = u
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
  file_base='multiple_timesteppers'
[]
