[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 15
    ny = 15
    xmin = 0
    ymin = 0
    xmax = 1
    ymax = 1.4
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = FALSE
[]

[Variables]
  [temperature]
  []
  [temperature_adjoint]
    solver_sys = adjoint
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
  # apply gradient material as a side force since the objective integral is only
  # over this side
  [top]
    type = MatNeumannBC
    boundary = top
    boundary_material = obj_misfit_gradient
    variable = temperature_adjoint
    value = 1
  []
[]

[Materials]
  [steel]
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
  # Create two materials.
  # 1. Material which the integral of is our objective
  # 2. dM/du material which is used for our adjoint problem
  [beam]
    type = MisfitReporterOffsetFunctionMaterial
    x_coord_name = measure_data/measurement_xcoord
    y_coord_name = measure_data/measurement_ycoord
    z_coord_name = measure_data/measurement_zcoord
    measurement_value_name = measure_data/measurement_values
    forward_variable = temperature
    property_name = obj_misfit
    function = gauss
  []
[]

[Functions]
  [gauss]
    type = ParsedFunction
    expression = 'exp(-2.0 *(x^2 + y^2 + z^2)/(beam_radii^2))'
    symbol_names = 'beam_radii'
    symbol_values = 0.1
  []
[]
[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[VectorPostprocessors]
  [gradient]
    type = PointValueSampler
    points = '0.2 1.1 0
    0.7 1.1 0
    0.4 1.1 0'
    variable = temperature_adjoint
    sort_by = id
    execute_on = ADJOINT_TIMESTEP_END
  []
[]

[Postprocessors]
  [objective]
    type = SideIntegralMaterialProperty
    boundary = top
    property = obj_misfit
    execute_on = 'TIMESTEP_END'
  []
  [largest_adjoint]
    type = NodalExtremeValue
    variable = temperature_adjoint
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
                          1.1 1.1 1.1;
                          0 0 0;
                          -1000 120 500'
  []
[]

[Outputs]
  console = false
[]
