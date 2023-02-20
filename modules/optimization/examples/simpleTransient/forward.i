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
  type = Transient

  num_steps = 100
  end_time = 1

  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  reuse_preconditioner=true
  reuse_preconditioner_max_linear_its=50
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
  csv=true
  console = false
[]
