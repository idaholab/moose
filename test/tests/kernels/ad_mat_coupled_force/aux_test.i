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
[]

[AuxVariables]
  [a]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = u
  []
  [force]
    type = ADMatCoupledForce
    variable = u
    v = a
    mat_prop_coef = test_prop
  []
[]

[AuxKernels]
  [a]
    variable = a
    type = ConstantAux
    value = 10
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 1
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
