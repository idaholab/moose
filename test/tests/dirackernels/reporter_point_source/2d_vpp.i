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
  inactive = 'reporter_point_source_err'
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
    value_name = 'reporterData2/u2'
    x_coord_name = 'reporterData1/x'
    y_coord_name = 'reporterData1/y'
    z_coord_name = 'reporterData1/z'
  []
  [reporter_point_source_err]
    type = ReporterPointSource
    variable = u
    value_name = 'reporterData2/u2'
    x_coord_name = 'reporterData2/x2'
    y_coord_name = 'reporterData1/y'
    z_coord_name = 'reporterData1/z'
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
  [reporterData1]
    type = ConstantReporter
    real_vector_names = 'x y z u'
    real_vector_values = '0.2 0.2 0.0; 0.3 0.8 0.0; 0 0 0; 5 5 5'
  []
  [reporterData2]
    type = ConstantReporter
    real_vector_names = 'x2 y2 z2 u2'
    real_vector_values = '0.2 0.2; 0.3 0.8 0.0; 0 0 0; 1 -.5 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
