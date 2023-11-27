[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
  uniform_refine = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[DiracKernels]
  inactive = 'reporter_point_source_err reporter_point_source_dup_err'
  [vpp_point_source]
    type = ReporterPointSource
    variable = u
    value_name = 'csv_reader/u'
    x_coord_name = 'csv_reader/x'
    y_coord_name = 'csv_reader/y'
    z_coord_name = 'csv_reader/z'
  []
  [reporter_point_source]
    type = ReporterPointSource
    variable = u
    value_name = 'reporterData/u'
    x_coord_name = 'reporterData/x'
    y_coord_name = 'reporterData/y'
    z_coord_name = 'reporterData/z'
    weight_name = 'reporterData/weight'
  []
  [reporter_point_source_err]
    type = ReporterPointSource
    variable = u
    value_name = 'reporterData_err/u2'
    x_coord_name = 'reporterData_err/x2'
    y_coord_name = 'reporterData_err/y2'
    z_coord_name = 'reporterData_err/z2'
  []
  [reporter_point_source_dup_err]
    type = ReporterPointSource
    variable = u
    value_name = 'reporterData_dup/u'
    x_coord_name = 'reporterData_dup/x'
    y_coord_name = 'reporterData_dup/y'
    z_coord_name = 'reporterData_dup/z'
    weight_name = 'reporterData_dup/weight'
    combine_duplicates=false
  []
  [reporter_point_source_dup]
    type = ReporterPointSource
    variable = u
    value_name = 'reporterData_dup/u'
    x_coord_name = 'reporterData_dup/x'
    y_coord_name = 'reporterData_dup/y'
    z_coord_name = 'reporterData_dup/z'
    weight_name = 'reporterData_dup/weight'
    combine_duplicates=true
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[VectorPostprocessors]
  [csv_reader]
    type = CSVReader
    csv_file = point_value_file.csv
  []
[]

[Reporters]
  [reporterData]
    type = ConstantReporter
    real_vector_names = 'x y z u weight'
    real_vector_values = '0.2 0.2 0.0; 0.3 0.8 0.0; 0 0 0; 1 -.5 0; 1 1 1'
  []
  [reporterData_err]
    type = ConstantReporter
    real_vector_names = 'x2 y2 z2 u2'
    real_vector_values = '0.2 0.2; 0.3 0.8 0.0; 0 0 0; 1 -.5 0'
  []
  [reporterData_dup]
    type = ConstantReporter
    real_vector_names = 'x y z u weight'
    real_vector_values = '0.2 0.2 0.2 0.0; 0.3 0.3 0.8 0.0; 0 0 0 0; 2 1 -.5 0;0.25 0.5  1 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
