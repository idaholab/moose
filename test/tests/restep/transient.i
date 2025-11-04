[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
  [src]
    type = BodyForce
    variable = u
    value = 1
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    variable = u
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
[]

restep_step = 1e12
[Postprocessors]
  [num_calls]
    type = GeneralSetupInterfaceCount
    count_type = INITIALIZE
  []
  [timestep]
    type = NumTimeSteps
  []
  [diff]
    type = ParsedPostprocessor
    expression = 'abs(num_calls - if(timestep < ${restep_step}, timestep, timestep + 1))'
    pp_names = 'num_calls timestep'
  []
  [diff_total]
    type = TimeIntegratedPostprocessor
    value = diff
  []
[]

[UserObjects]
  [terminate]
    type = Terminator
    expression = 'diff_total > 1e-12'
    error_level = ERROR
    message = 'Restep did not occur when expected.'
    execute_on = 'FINAL'
  []
[]
