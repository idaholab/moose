[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 4
  nz = 2
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 0.2
  elem_type = HEX20
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./square_planar_cut_uo]
    type = RectangleCutUserObject
    cut_data = '  0.35 1.01 -0.001
                  0.35 0.49 -0.001
                  0.35 0.49  0.201
                  0.35 1.01  0.201'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./u_left]
    type = PiecewiseLinear
    x = '0   2'
    y = '0  0.1'
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
  [./left_u]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = u_left
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
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
