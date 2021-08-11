[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 1
  ymax = 1.4
[]

[Variables]
  [temperature]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [saved_t]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
    save_in = saved_t
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = temperature
    x_coord_name = 'point_source/x'
    y_coord_name = 'point_source/y'
    z_coord_name = 'point_source/z'
    value_name = 'point_source/value'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 300
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 300
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 300
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 300
  []
[]

[Materials]
  [steel]
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [point_source]
    type = ConstantVectorPostprocessor
    vector_names = 'x y z value'
    value = '0.2 0.2 0.8;
             0.2 0.8 0.2;
             0 0 0;
             -2458 7257 26335'
  []
  #-----every forward problem should have these two
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
    # x_coord_name = measure_data/measurement_xcoord
    # y_coord_name = measure_data/measurement_ycoord
    # z_coord_name = measure_data/measurement_zcoord
  []
  [vertical]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0.5 0 0'
    end_point = '0.5 1.4 0'
    num_points = 21
    sort_by = y
  [../]
[]

[Reporters]
  [measure_data]
    type=OptimizationData
  []
[]
#---------------------------------------------------

[Outputs]
  csv=true
  json=false
  console = true
  exodus = false
<<<<<<< HEAD:test/tests/optimizationreporter/objective_gradient_minimize/point_loads/forward.i
=======
  csv=true
>>>>>>> 9811d3b (updated tests to output the point load figures included in the ldrd seed poster and report):test/tests/formfunction/objective_gradient_minimize/point_loads/forward.i
  file_base = 'forward'
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
  print_linear_residuals = false
[]
