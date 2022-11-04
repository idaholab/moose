[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
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
  [./line_seg_cut_uo]
    type = LineSegmentCutSetUserObject
    cut_data = '0.3 1.0 0.3 0.2 0 3'
    heal_always = false
  [../]
  [./level_set_cut_uo]
    type = LevelSetCutUserObject
    level_set_var = ls
    heal_always = true
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./ls]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./ls_function]
    type = FunctionAux
    variable = ls
    function = ls_func
  [../]
[]

[Functions]
  [./u_left]
    type = PiecewiseLinear
    x = '0   2'
    y = '3   5'
  [../]
  [./ls_func]
    type = ParsedFunction
    expression = 'x-0.7-0.07*(t-1)'
  [../]
[]

[Constraints]
  [./u_constraint]
    type = XFEMSingleVariableConstraint
    geometric_cut_userobject = 'level_set_cut_uo'
    use_displaced_mesh = false
    variable = u
    use_penalty = true
    alpha = 1e5
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

[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  # petsc_options_iname = '-pc_type -pc_hypre_type'
  # petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'

  l_tol = 1e-3
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-9

  start_time = 0.0
  dt = 1
  end_time = 3.0
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
