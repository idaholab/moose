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

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [temperature]
  []
  [temperature_adjoint]
    nl_sys = adjoint
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
  [misfit]
    type = ReporterPointSource
    variable = temperature_adjoint
    x_coord_name = measure_data/measurement_xcoord
    y_coord_name = measure_data/measurement_ycoord
    z_coord_name = measure_data/measurement_zcoord
    value_name = measure_data/misfit_values
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
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  nl_rel_tol = 1e-12
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[VectorPostprocessors]
  [gradient]
    type = PointValueSampler
    points = '0.2 0.2 0
              0.7 0.56 0
              0.4 1 0'
    variable = temperature_adjoint
    sort_by = id
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    variable = temperature
  []
  [point_source]
    type = ConstantReporter
    real_vector_names = 'x y z value'
    real_vector_values = '0.2 0.7 0.4;
                          0.2 0.56 1;
                          0 0 0;
                          -1000 120 500'
  []
[]

[Outputs]
  console = false
[]
