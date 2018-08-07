# The Neumann BC is applied on the cutted boundary.
# The solution is not correct because so far integration along the cutted element faces is not right.
# To correct this, we need to re-calcuate the weights based on area/volume fraction. This will be implemented soon.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 6
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  elem_type = QUAD4
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./line_seg_cut_uo]
    type = LineSegmentCutUserObject
    cut_data = '0.5  1.0  0.5  0.5'
    time_start_cut = 0.0
    time_end_cut = 0.0
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
# Define boundary conditions
  [./top_u]
    type = NeumannBC
    variable = u
    boundary = 2
    value = -1.0
  [../]

  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 0
    value = 0
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
  dt = 1.0
  end_time = 1.0
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
