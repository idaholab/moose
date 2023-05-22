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
  type = Transient   # Here we use the Transient Executioner
  [TimeSteppers]
      type = TimeSequenceStepper
      time_sequence  = '0 43200 86400 172800 432000 864000'
  []
  start_time = 0.0
  end_time = 864000
[]

[Postprocessors]
  [timestep]
    type = TimePostprocessor
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  file_base='multiple_timesequence'
[]
