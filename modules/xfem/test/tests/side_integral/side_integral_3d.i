[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 1
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 0.2
  elem_type = HEX8
[]

[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [./square_cut_uo]
    type = RectangleCutUserObject
    cut_data = ' -1.0 -0.1 -1.0
                  2.0  1.1 -1.0
                  2.0  1.1  1.0
                 -1.0 -0.1  1.0'
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
  [./front]
    type = DirichletBC
    variable = u
    boundary = front
    value = 3
  [../]

  [./back]
    type = DirichletBC
    variable = u
    boundary = back
    value = 2
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

[Postprocessors]
  [./front]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = front
  [../]
  [./back]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = back
  [../]
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
