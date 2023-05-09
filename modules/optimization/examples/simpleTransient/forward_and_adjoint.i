[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
  []
[]

[Variables]
  [u]
  []
[]

[VectorPostprocessors]
  [src_values]
    type = CSVReader
    csv_file = source_params.csv
    header = true
    outputs = none
  []
[]

[ICs]
  [initial]
    type = FunctionIC
    variable = u
    function = exact
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
  [src]
    type = BodyForce
    variable = u
    function = source
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    value = '2*exp(-2.0*(x - sin(2*pi*t))^2)*exp(-2.0*(y - cos(2*pi*t))^2)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/pi'
  []
  [source]
    type = NearestReporterCoordinatesFunction
    x_coord_name = src_values/coordx
    y_coord_name = src_values/coordy
    time_name = src_values/time
    value_name = src_values/values
  []
[]

[Executioner]
  type = TransientAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint

  num_steps = 100
  end_time = 1

  nl_rel_tol = 1e-12
  l_tol = 1e-12
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Reporters]
  [measured_data]
    type = OptimizationData
    measurement_file = mms_data.csv
    file_xcoord = x
    file_ycoord = y
    file_zcoord = z
    file_time = t
    file_value = u
    variable = u
    execute_on = timestep_end
    outputs = none
  []
[]

[Postprocessors]
  [topRight_pp]
    type = PointValue
    point = '0.5 0.5 0'
    variable = u
    execute_on = TIMESTEP_END
  []
  [bottomRight_pp]
    type = PointValue
    point = '-0.5 0.5 0'
    variable = u
    execute_on = TIMESTEP_END
  []
  [bottomLeft_pp]
    type = PointValue
    point = '-0.5 -0.5 0'
    variable = u
    execute_on = TIMESTEP_END
  []
  [topLeft_pp]
    type = PointValue
    point = '0.5 -0.5 0'
    variable = u
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  csv = true
  console = false
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [u_adjoint]
    nl_sys = adjoint
    outputs = none
  []
[]

[DiracKernels]
  [misfit]
    type = ReporterTimePointSource
    variable = u_adjoint
    value_name = measured_data/misfit_values
    x_coord_name = measured_data/measurement_xcoord
    y_coord_name = measured_data/measurement_ycoord
    z_coord_name = measured_data/measurement_zcoord
    time_name = measured_data/measurement_time
  []
[]

[VectorPostprocessors]
  [adjoint]
    type = ElementOptimizationSourceFunctionInnerProduct
    variable = u_adjoint
    function = source
    execute_on = ADJOINT_TIMESTEP_END
    outputs = none
  []
[]
