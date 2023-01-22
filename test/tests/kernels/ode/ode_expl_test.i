[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 5
  ny = 5
  elem_type = QUAD4
[]

[Functions]
  [./f_fn]
    type = ParsedFunction
    expression = -4
  [../]
  [./bc_all_fn]
    type = ParsedFunction
    expression = x*x+y*y
  [../]
[]

# NL

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./uff]
    type = BodyForce
    variable = u
    function = f_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = bc_all_fn
  [../]
[]

# Aux

[AuxVariables]
  [./y]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
[]

[AuxScalarKernels]
  [./ode1]
    type = ExplicitODE
    variable = y
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 0.1
  num_steps = 10
[]

[Outputs]
  exodus = true
[]
