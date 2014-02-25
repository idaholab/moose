[Mesh]
 	 type = GeneratedMesh
 	 dim = 2
 	 xmin = -1
 	 ymin = -1
 	 xmax = 1
 	 ymax = 1
   nx = 2
   ny = 2
   elem_type = QUAD4
[]

[Functions]
  [./fn_exact]
    type = ParsedFunction
    value = 'x*x+y*y'
  [../]

  [./ffn]
    type = ParsedFunction
    value = -4
  [../]
[]

[UserObjects]
  [./buo]
    type = BoundaryUserObject
    boundary = 'bottom top left right'
    variable = u
    factors = '1.0 1.0 1.0 1.0'
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = UserForcingFunction
    variable = u
    function = ffn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = fn_exact
  [../]
[]

[Postprocessors]
  [./value]
    type = BoundaryValuePPS
    user_object = buo
  [../]
[]

[Executioner]
  type = Steady
[]

[Output]
  output_initial = false
  exodus = true
[]
