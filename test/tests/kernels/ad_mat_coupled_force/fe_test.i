[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 4
    ny = 4
    dim = 2
  []
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
  []
  [force_u]
    type = ADMatCoupledForce
    variable = u
    v = v
    mat_prop_coef = test_prop
  []

  [diff_v]
    type = ADDiffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []

  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 5
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 3
  []
[]

[Functions]
  [test_func]
    type = ParsedFunction
    expression = 'x'
  []
[]

[Materials]
  [test_prop]
    type = ADGenericFunctionMaterial
    prop_names = test_prop
    prop_values = test_func
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
