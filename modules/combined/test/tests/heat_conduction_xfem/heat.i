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
  geometric_cut_userobjects = 'line_seg_cut_uo'
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
  [./temp]
    initial_condition = 300.0     # set initial temp to ambient
  [../]
[]

[Functions]
  [./temp_left]
    type = PiecewiseLinear
    x = '0   2'
    y = '0  0.1'
  [../]
[]

[Kernels]
  [./heat]         # gradient term in heat conduction equation
    type = HeatConduction
    variable = temp
  [../]
[]

[BCs]
# Define boundary conditions
  [./left_temp]
    type = FunctionDirichletBC
    variable = temp
    boundary = 3
    function = temp_left
  [../]

  [./right_temp]
    type = DirichletBC
    variable = temp
    boundary = 1
    value = 0
  [../]
[]

[Materials]
  [./fuel_thermal]
    type = HeatConductionMaterial
    block = 0
    temp = temp
    thermal_conductivity = 5.0
    specific_heat = 1.0
  [../]
[]

[Executioner]

  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      8'

  line_search = 'none'

  l_max_its = 100
  l_tol = 8e-3

  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 1.0
  end_time = 2.0
  num_steps = 2
[]

[Outputs]
  # Define output file(s)

  file_base = heat_out
  interval = 1
  execute_on = timestep_end
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
