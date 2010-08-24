[Mesh]
  dim = 2
  generated = true
  [./Generation]
   nx = 5
   ny = 5
   xmin = -1 
   xmax = 1
   ymin = -1
   ymax = 1
   elem_type = QUAD4
  [../]
[]

[Variables]
  active = 'u'

  [./u] 
    order = SECOND 
    family = MONOMIAL

	[./InitialCondition]
      type = ConstantIC
      value = 1 
	[../]
  [../]
[]

[Functions]
  active = 'forcing_fn exact_fn'
  
  [./forcing_fn]
    type = ParsedFunction
#    function = -4.0+(x*x)+(y*y)
#    function = x
#    function = (x*x)-2.0
#    function = -pow(e,-x-(y*y))*(4*y*y-2)
    function = (x*x*x)-6.0*x
  [../]
  
  [./exact_fn]
    type = ParsedGradFunction
#    function = x
#    grad_x = 1
#    grad_y = 0
    
#    function = (x*x)+(y*y)
#    grad_x = 2*x
#    grad_y = 2*y

#    function = (x*x)
#    grad_x = 2*x
#    grad_y = 0

#    function = pow(e,-x-(y*y))
#    grad_x = -pow(e,-x-(y*y))
#    grad_y = -2*y*pow(e,-x-(y*y))

    function = (x*x*x)
    grad_x = 3*x*x
    grad_y = 0
  [../]
[]

[Kernels]
  active = 'diff abs forcing'

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
  [../]
[]

[DGKernels]
  active = 'dg_diff'
  
  [./dg_diff]
  	type = DGDiffusion
  	variable = u
  	epsilon = 0
  	sigma = 18
  [../]
[]

[BCs]
  active = 'all'

  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
	epsilon = 0
	sigma = 18
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
  petsc_options = '-snes_mf_operator'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre    boomeramg'
  
#  petsc_options = '-snes_mf'
  max_r_steps = 0
  [./Adaptivity]
  	steps = 1
    refine_fraction = 1.0
    coarsen_fraction = 0
    max_h_level = 8
  [../]
  
#  nl_rel_tol = 1e-12
[]

[Postprocessors]
  active = 'h dofs l2_err'

  [./h]
    type = AverageElementSize
    variable = u
  [../]
  
  [./dofs]
    type = PrintDOFs
    variable = u
  [../]
  
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]  
[]

[Output]
  file_base = out
  output_initial = false
  interval = 1
  exodus = true
  postprocessor_csv = true
[]
