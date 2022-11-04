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
    expression = t*x
  [../]
  [./ffn]
    type = ParsedFunction
    expression = x
  [../]
[]

[Variables]
  [./u]
    initial_condition = 4.2
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
    type = BodyForce
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
