# 2D: Mesh is cut by level set based cutter
# The level set is a MOOSE variable

[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./level_set_cut_uo]
    type = LevelSetCutUserObject
    level_set_var = ls
  [../]
[]

[Variables]
  [./u]
  [../]
  [./ls]
  [../]
[]

[Functions]
  [./u_left]
    type = PiecewiseLinear
    x = '0   2'
    y = '3   5'
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./diff_ls]
    type = Diffusion
    variable = ls
  [../]
[]

[BCs]
# Define boundary conditions
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 3
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./left_ls]
    type = DirichletBC
    variable = ls
    boundary = 3
    value = 3
  [../]

  [./right_ls]
    type = DirichletBC
    variable = ls
    boundary = 1
    value = -3
  [../]

[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'

  l_tol = 1e-3
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 1
  end_time = 1.0
  max_xfem_update = 1
[]

[Outputs]
  interval = 1
  execute_on = timestep_end
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
