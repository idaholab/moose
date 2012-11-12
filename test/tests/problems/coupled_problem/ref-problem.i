[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 20
  ny = 10
  elem_type = QUAD9
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    value = 4
  [../]
  [./bc_fn]
    type = ParsedFunction
    value = -(x*x+y*y)
  [../]
  
  [./bc_fn_v]
    type = ParsedFunction
    value = (x*x+y*y)
  [../]

  
  [./ffnv]
    type = ParsedFunction
    value = -1
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
  [./v]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[Kernels]
  # U equation
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./forcing_fn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
  
  # V equation
  [./td_v]
    type = TimeDerivative
    variable = v
  [../]
  [./diff_v]
    type = CoefDiffusion
    variable = v
    coef = 0.5
  [../]
  [./conv_v]
    type = CoupledConvection
    variable = v
    velocity_vector = u
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = bc_fn
  [../]

  [./left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = 'top'
    function = bc_fn_v
  [../]
[]

[Executioner]
  type = Transient
  # get 'er done
#  nl_rel_tol = 1e-15
#  nl_abs_tol = 1e-14
  
  start_time = 0
  dt = 0.05
  num_steps = 10
[]

[Output]
  output_initial = false
  exodus = true
[]
