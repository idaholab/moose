[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./a]
    family = SCALAR
    order = FIRST
  [../]
[]

[Functions]
  [./a_fn]
    type = ParsedFunction
    expression = '4 - t'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxScalarKernels]
  [./a_sk]
    type = FunctionScalarAux
    variable = a
    function = a_fn
    execute_on = 'initial timestep_begin'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 2
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 3
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-10
  dt = 1
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
