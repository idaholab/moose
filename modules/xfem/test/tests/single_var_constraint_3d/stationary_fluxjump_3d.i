[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 2
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 0.25
  elem_type = HEX8
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./square_planar_cut_uo]
    type = RectangleCutUserObject
    cut_data = ' 0.5 -0.001 -0.001
                 0.5  1.001 -0.001
                 0.5  1.001  1.001
                 0.5 -0.001  1.001'
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

[Constraints]
  [./xfem_constraint]
    type = XFEMSingleVariableConstraint
    variable = u
    jump = 0
    jump_flux = 1
    geometric_cut_userobject = 'square_planar_cut_uo'
  [../]
[]

[BCs]
# Define boundary conditions
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
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
  end_time = 2.0
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
