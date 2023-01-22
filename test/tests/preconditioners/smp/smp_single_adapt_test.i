#
# This is not very strong test since the problem being solved is linear, so the difference between
# full Jacobian and block diagonal preconditioner is not that big
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 5
  ny = 5
  elem_type = QUAD4
[]

[Functions]
  [./exact_v]
    type = ParsedFunction
    expression = sin(pi*x)*sin(pi*y)
  [../]
  [./force_fn_v]
    type = ParsedFunction
    expression = 2*pi*pi*sin(pi*x)*sin(pi*y)
  [../]
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

[Preconditioning]
  [./SMP]
    type = SMP
    off_diag_row    = 'u'
    off_diag_column = 'v'
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./conv_u]
    type = CoupledForce
    variable = u
    v = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./ffn_v]
    type = BodyForce
    variable = v
    function = force_fn_v
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]

  [./all_v]
    type = FunctionDirichletBC
    variable = v
    boundary = '0 1 2 3'
    function = exact_v
  [../]
[]

[Executioner]
  type = Steady


  solve_type = 'PJFNK'

  [./Adaptivity]
    steps = 3
    coarsen_fraction = 0.1
    refine_fraction = 0.2
    max_h_level = 5
  [../]
[]

[Outputs]
  exodus = true
  print_mesh_changed_info = true
[]
