# This example reproduces the libmesh vector_fe example 1 results

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmin = -1
  ymin = -1
[]

[Variables]
  [u]
    family = LAGRANGE_VEC
  []
[]

[AuxVariables]
  [mag]
    order = FIRST
    family = MONOMIAL
  []
[]

[AuxKernels]
  [mag]
    type = ADFunctorElementalAux
    variable = mag
    functor = mat_mag
  []
[]

[Kernels]
  [diff]
    type = VectorDiffusion
    variable = u
  []
  [body_force]
    type = VectorBodyForce
    variable = u
    function_x = 'ffx'
    function_y = 'ffy'
  []
[]

[BCs]
  [bnd]
    type = VectorFunctionDirichletBC
    variable = u
    function_x = 'x_exact_sln'
    function_y = 'y_exact_sln'
    function_z = '0'
    boundary = 'left right top bottom'
  []
[]

[Functions]
  [x_exact_sln]
    type = ParsedFunction
    expression = 'cos(.5*pi*x)*sin(.5*pi*y)'
  []
  [y_exact_sln]
    type = ParsedFunction
    expression = 'sin(.5*pi*x)*cos(.5*pi*y)'
  []
  [ffx]
    type = ParsedFunction
    expression = '.5*pi*pi*cos(.5*pi*x)*sin(.5*pi*y)'
  []
  [ffy]
    type = ParsedFunction
    expression = '.5*pi*pi*sin(.5*pi*x)*cos(.5*pi*y)'
  []
[]

[Materials]
  [functor]
    type = ADVectorMagnitudeFunctorMaterial
    vector_functor = u
    vector_magnitude_name = mat_mag
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
