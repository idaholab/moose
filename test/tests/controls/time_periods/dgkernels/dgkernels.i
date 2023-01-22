[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Adaptivity]
  marker = uniform_marker
  [./Markers]
    [./uniform_marker]
      type = UniformMarker
      mark = REFINE
    [../]
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL
    initial_condition = 1
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = 2*pow(e,-x-(y*y))*(1-2*y*y)
  [../]

  [./exact_fn]
    type = ParsedGradFunction
    value = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./abs]          # u * v
    type = Reaction
    variable = u
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[DGKernels]
  [./dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  [../]
  [./dg_diff2]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 4
  [../]
[]

[BCs]
  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre    boomeramg'
  num_steps = 4
  dt = 1
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
[]

[Controls]
  [./dg_problem]
    type = TimePeriod
    enable_objects = 'DGKernels/dg_diff2'
    disable_objects = 'DGKernel::dg_diff'
    start_time = '2'
    execute_on = 'initial timestep_begin'
  [../]
[]
