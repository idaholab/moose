# DO NOT CHANGE THIS TEST
# this test is documented as an example in forceInv_pointLoads.md
# if this test is changed, the figures will need to be updated.
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 1
    ymax = 1.4
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = temperature
    diffusivity = thermal_conductivity
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
  solve_type = NEWTON
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[VectorPostprocessors]
  [point_source]
    type = ConstantVectorPostprocessor
    vector_names = 'x y z value'
    value = '0.2 0.7 0.4;
             0.2 0.56 1;
             0 0 0;
             -1000 120 500'
    execute_on = LINEAR
  []
  [vertical]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0.5 0 0'
    end_point = '0.5 1.4 0'
    num_points = 21
    sort_by = y
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
  []
[]
#---------------------------------------------------

[Outputs]
  console = false
  file_base = 'forward'
[]
