# This example reproduces the libmesh vector_fe example 1 results

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 15
  xmin = -1
  elem_type = EDGE3
[]

[Variables]
  [./u]
    family = LAGRANGE_VEC
    order = SECOND
  [../]
[]

[Kernels]
  [./diff]
    type = VectorDiffusion
    variable = u
  [../]
  [./body_force]
    type = VectorBodyForce
    variable = u
    function_x = 'ffx'
  [../]
[]

[BCs]
  [./bnd]
    type = VectorFunctionDirichletBC
    variable = u
    function_x = 'x_exact_sln'
    boundary = 'left right'
  [../]
[]

[Functions]
  [./x_exact_sln]
    type = ParsedFunction
    expression = 'cos(.5*pi*x)'
  [../]
  [./ffx]
    type = ParsedFunction
    expression = '.25*pi*pi*cos(.5*pi*x)'
  [../]
[]

[Preconditioning]
  [./pre]
    type = SMP
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
