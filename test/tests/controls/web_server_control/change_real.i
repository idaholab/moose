# Change the controllable value of BCs/left/value every timestep
# via the WebServerControl

[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 10
  ny = 10
[]

[Variables/u]
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
    value = 0 # Changed by Controls/web_server
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
[]

[Outputs]
  exodus = true
[]

[Controls/web_server]
  type = WebServerControl
  port = 8000 # will get overridden by the script to find an available port
  execute_on = TIMESTEP_BEGIN
[]
