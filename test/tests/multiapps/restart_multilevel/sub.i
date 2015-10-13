[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
[]

[Functions]
  [./u_fn]
    type = ParsedFunction
    value = t*x
  [../]
  [./ffn]
    type = ParsedFunction
    value = x
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./v]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./fn]
    type = UserForcingFunction
    variable = u
    function = ffn
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = u_fn
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.1
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub_app]
    app_type = MooseTestApp
    type = TransientMultiApp
    input_files = 'subsub.i'
    execute_on = timestep_end
    positions = '0 -1 0'
  [../]
[]

[Transfers]
  [./from_sub]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub_app
    source_variable = u
    variable = v
  [../]
[]
