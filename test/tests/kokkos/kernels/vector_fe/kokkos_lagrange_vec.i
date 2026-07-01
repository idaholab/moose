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
  [u]
    family = LAGRANGE_VEC
    order = SECOND
  []
[]

[Kernels]
  [diff]
    type = KokkosVectorDiffusion
    variable = u
  []
  [body_force]
    type = KokkosVectorBodyForce
    variable = u
    function_x = 'ffx'
    function_y = 'ffy'
  [../]
[]

[BCs]
  [bnd]
    type = KokkosVectorFunctionDirichletBC
    variable = u
    function_x = 'x_exact_sln'
    function_y = 'y_exact_sln'
    boundary = 'left right top bottom'
  []
[]

[Functions]
  [x_exact_sln]
    type = KokkosParsedFunction
    expression = 'cos(.5*pi*x)*sin(.5*pi*y)'
  []
  [y_exact_sln]
    type = KokkosParsedFunction
    expression = 'sin(.5*pi*x)*cos(.5*pi*y)'
  []
  [ffx]
    type = KokkosParsedFunction
    expression = '.5*pi*pi*cos(.5*pi*x)*sin(.5*pi*y)'
  []
  [ffy]
    type = KokkosParsedFunction
    expression = '.5*pi*pi*sin(.5*pi*x)*cos(.5*pi*y)'
  []
[]

[Preconditioning]
  [pre]
    type = SMP
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
