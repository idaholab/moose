[Mesh]
  dim = 2
  generated = true
  [./Generation]
   nx = 5
   ny = 5
   xmin = 0 
   xmax = 1
   ymin = 0
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
#    value = -4.0+(x*x)+(y*y)
#    value = x
#    value = (x*x)-2.0
#    value = -pow(e,-x-(y*y))*(4*y*y-2)
    value = -6.0*(x*x)-6.0*x
#    value = -6.0*x
  [../]
  
  [./exact_fn]
    type = ParsedGradFunction
#    value = x
#    grad_x = 1
#    grad_y = 0
    
#    value = (x*x)+(y*y)
#    grad_x = 2*x
#    grad_y = 2*y

#    value = (x*x)
#    grad_x = 2*x
#    grad_y = 0

#    value = pow(e,-x-(y*y))
#    grad_x = -pow(e,-x-(y*y))
#    grad_y = -2*y*pow(e,-x-(y*y))

    value = (x*x*x)
    grad_x = 3*x*x
    grad_y = 0
  [../]
[]

[Kernels]
  active = 'diff conv forcing'
#  active = 'diff forcing'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./conv]
    type = Convection
    variable = u
    x = -2.0
    y = 0.0
  [../]
  
  
  [./forcing]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[DGKernels]
  active = 'dg_diff dg_conv'
#  active = 'dg_diff'
  
  [./dg_diff]
  	type = DGDiffusion
  	variable = u
  	epsilon = 1
  	sigma = 6
  [../]

  [./dg_conv]
        type = DGConvection
        variable = u
        x = -2.0
        y = 0.0
  [../]
[]

[BCs]
  active = 'all plus'
#  active = 'all'

  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
	  epsilon = 1
  	sigma = 6
  [../]

  [./plus]
    type = DGFunctionConvectionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
      x = -2.0
      y = 0.0
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
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
  
#  petsc_options = '-snes_mf'
  max_r_steps = 6
  [./Adaptivity]
  	steps = 1
    refine_fraction = 1.0
    coarsen_fraction = 0
    max_h_level = 5
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
