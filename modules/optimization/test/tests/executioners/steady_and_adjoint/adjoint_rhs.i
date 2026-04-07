[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.01022
    xmax = 0
    ymin = 0
    ymax = 0.01016
    zmin = -0.00349
    zmax = 0
    nx = 11
    ny = 11
    nz = 3
  []
  [pin]
    type = ExtraNodesetGenerator
    input = gen
    new_boundary = pin
    coord = '-0.01022 0.01016 -0.00349'
    use_closest_node = true
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [temperature]
    initial_condition = 1.0e-8
  []
  [temperature_adj]
    solver_sys = adjoint
    initial_condition = 1.0e-8
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = temperature
  []
[]

[BCs]
  [left_bc]
    type = DirichletBC
    boundary = pin
    value = 0
    variable = temperature
  []
  [flux]
    type = FunctionNeumannBC
    variable = temperature
    boundary = front
    function = '10*1/0.00025/sqrt(2*pi)*exp((-1/2)*((x+0.002914666)^2+(y-0.005110557)^2)/0.00025^2)'
  []
[]

[Postprocessors]
  [ref_value]
    type = PointValue
    point = '-0.002404677 0.005612893 0'
    variable = temperature
    execute_on = TIMESTEP_END
    execution_order_group = 0
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
    measurement_file = measurements_440.csv
    file_xcoord = x
    file_ycoord = y
    file_zcoord = z
    file_value = value
    variable = temperature
    execute_on = TIMESTEP_END
    execution_order_group = 0
  []
  [phase_reference_location]
    type = ConstantReporter
    real_vector_names = 'x y z'
    real_vector_values = '-0.002404677; 0.005612893; 0'
    execute_on = TIMESTEP_END
    execution_order_group = 0
  []
  [misfit]
    type = ParsedVectorReporter
    name = value
    vector_reporter_names = 'measure_data/measurement_values measure_data/simulation_values'
    vector_reporter_symbols = 'measured simulation'
    scalar_reporter_names = 'ref_value/value'
    scalar_reporter_symbols = 'ref'
    expression = 'simulation-measured-ref'
    execute_on = TIMESTEP_END
    execution_order_group = 1
  []
  [measurement_source]
    type = ParsedVectorReporter
    name = value
    vector_reporter_names = 'misfit/value'
    vector_reporter_symbols = 'misfit'
    expression = 'misfit'
    execute_on = TIMESTEP_END
    execution_order_group = 2
  []
  [misfit_sum]
    type = ParsedVectorRealReductionReporter
    name = value
    vector_reporter_name = misfit/value
    initial_reduction_value = 0
    expression = 'reduction_value+indexed_value'
    execute_on = TIMESTEP_END
    execution_order_group = 2
  []
  [reference_source]
    type = ParsedVectorReporter
    name = value
    vector_reporter_names = 'phase_reference_location/x'
    vector_reporter_symbols = 'x'
    scalar_reporter_names = 'misfit_sum/value'
    scalar_reporter_symbols = 'misfit_sum'
    expression = '-misfit_sum'
    execute_on = TIMESTEP_END
    execution_order_group = 3
  []
[]

[DiracKernels]
  [measurement_dirac]
    type = ReporterPointSource
    variable = temperature_adj
    x_coord_name = measure_data/measurement_xcoord
    y_coord_name = measure_data/measurement_ycoord
    z_coord_name = measure_data/measurement_zcoord
    value_name = measurement_source/value
  []
  [reference_dirac]
    type = ReporterPointSource
    variable = temperature_adj
    x_coord_name = phase_reference_location/x
    y_coord_name = phase_reference_location/y
    z_coord_name = phase_reference_location/z
    value_name = reference_source/value
  []
[]

[Preconditioning]
  [nl0]
    type = SMP
    nl_sys = nl0
    petsc_options_iname = '-ksp_type -pc_type -pc_gamg_type -pc_gamg_threshold -mg_levels_ksp_type -mg_levels_pc_type -ksp_gmres_restart'
    petsc_options_value = 'gmres asm agg 0.05 chebyshev jacobi 201'
  []
  [adjoint]
    type = SMP
    nl_sys = adjoint
    petsc_options_iname = '-ksp_type -pc_type -pc_gamg_type -pc_gamg_threshold -mg_levels_ksp_type -mg_levels_pc_type -ksp_gmres_restart'
    petsc_options_value = 'gmres asm agg 0.05 chebyshev jacobi 201'
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  automatic_scaling = true
[]
