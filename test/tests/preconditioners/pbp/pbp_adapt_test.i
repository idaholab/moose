[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = -4
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = ((x*x)+(y*y))
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]

  [./conv_v]
    type = CoupledForce
    variable = v
    v = u
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 0
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Preconditioning]
  [./PBP]
    type = PBP
    solve_order = 'u v'
    preconditioner  = 'AMG ASM'
    off_diag_row    = 'v'
    off_diag_column = 'u'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = JFNK

  [./Adaptivity]
    steps = 3
    coarsen_fraction = 0.1
    refine_fraction = 0.2
    max_h_level = 5
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_pbp_adapt
  print_mesh_changed_info = true
  exodus = true
[]
