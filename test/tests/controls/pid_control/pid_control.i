c = 0

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = '${c}'
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  start_time = 0.0
  end_time = 20
  dt = 1
[]

[Postprocessors]
  [integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'initial timestep_end'
  []
[]

[Controls]
  [integral_value]
    type = PIDTransientControl
    postprocessor = integral
    target = 1.5
    parameter = 'BCs/left/value'
    K_integral = -1
    K_proportional = -1
    K_derivative = -0.1
    execute_on = 'initial timestep_begin'
  []
[]

[Outputs]
  file_base = out
  exodus = false
  csv = true
[]
