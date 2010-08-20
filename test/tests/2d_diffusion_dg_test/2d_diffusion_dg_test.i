[Mesh]
  dim = 2
#  generated = true
#  [./Generation]
#   nx = 10
#   ny = 10
##   nx = 5
##   ny = 5
#   xmin = -1 
#   xmax = 1
#   ymin = -1
#   ymax = 1
#   elem_type = QUAD4
#  [../]

  file = l-shape.e
  uniform_refine = 2
[]

[Variables]
  active = 'u'

  [./u] 
#    order = CONSTANT
    order = FIRST
#    order = SECOND
#    order = THIRD
    
    family = MONOMIAL
#    family = LAGRANGE
#    family = SCALAR
#    family = XYZ
#    family = HIERARCHIC

	[./InitialCondition]
      type = ConstantIC
      value = 1 
	[../]
  [../]
[]

[Functions]
#  active = ''
#  active = 'br_forcing_fn br_exact'

  active = 'forcing_fn exact_fn br_forcing_fn br_exact'
  
  [./forcing_fn]
    type = ParsedFunction
#    function = -4.0+(x*x)+(y*y)
    function = x
#    function = (x*x)-2.0
  [../]
  
  [./exact_fn]
    type = ParsedGradFunction
    function = x
    grad_x = 1
    grad_y = 0
    
#    function = (x*x)+(y*y)
#    grad_x = 2*x
#    grad_y = 2*y

#    function = (x*x)
#    grad_x = 2*x
#    grad_y = 0
  [../]
  
  # BR
  [./br_forcing_fn]
    type = ParsedFunction
    function = -pow(e,-x-(y*y))*(4*y*y-2)
  [../]
  
  [./br_exact]
    type = ParsedGradFunction
    function = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))
  [../]
[]

[Kernels]
  active = 'diff'
#  active = 'diff abs forcing'
#  active = 'abs forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]
  
  [./abs]					# u * v
    type = Reaction
    variable = u
  [../]
  
  [./forcing]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
#    function = br_forcing_fn
  [../]
[]

[DGKernels]
  active = 'nipg0'
#  active = ' '
  
  [./nipg0]
  	type = NIPG0
  	variable = u
  	e = -1
  	sigma = 16
  [../]
[]

[BCs]
  active = 'all'

  [./all]
#    type = FunctionNeumannBC
#    type = FunctionDirichletBC
    type = DGBC
#    type = NeumannBC
    variable = u
#    boundary = '0 1 2 3'
#    boundary = '1 3'
    boundary = '1'
    function = exact_fn
#    function = br_exact
#	value = 0
    pen = 16
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
#  petsc_options = '-snes_mf_operator'
  petsc_options = '-snes_mf'
  max_r_steps = 0
  [./Adaptivity]
  	steps = 1
    refine_fraction = 1.0
    coarsen_fraction = 0
    max_h_level = 8
  [../]
[]

[Postprocessors]
  active = 'h dofs l2_err'

  [./h]
    type = AverageElementSize
    variable = u					# kinda stupid, but what can we do :)
  [../]
  
  [./dofs]
    type = PrintDOFs
    variable = u
  [../]
  
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
#    function = br_exact
  [../]  
[]

[Output]
  file_base = out
  output_initial = false
  interval = 1
  exodus = true
  postprocessor_csv = true
[]
