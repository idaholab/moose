# This example reproduces the libmesh vector_fe example 1 results

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmin = -1
  ymin = -1
  elem_type = QUAD9
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
    function_y = 'ffy'
  [../]
[]

[BCs]
  [./bnd]
    type = LagrangeVecFunctionDirichletBC
    variable = u
    x_exact_soln = 'x_exact_sln'
    y_exact_soln = 'y_exact_sln'
    z_exact_soln = '0'
    boundary = 'left right top bottom'
  [../]
[]

[Functions]
  [./x_exact_sln]
    type = ParsedFunction
    value = 'cos(.5*pi*x)*sin(.5*pi*y)'
  [../]
  [./y_exact_sln]
    type = ParsedFunction
    value = 'sin(.5*pi*x)*cos(.5*pi*y)'
  [../]
  [./ffx]
    type = ParsedFunction
    value = '.5*pi*pi*cos(.5*pi*x)*sin(.5*pi*y)'
  [../]
  [./ffy]
    type = ParsedFunction
    value = '.5*pi*pi*sin(.5*pi*x)*cos(.5*pi*y)'
  [../]
[]

[Preconditioning]
  [./pre]
    type = SMP
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
