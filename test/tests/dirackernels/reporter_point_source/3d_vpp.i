[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  active = 'u'
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
  [point_source]
    type = ReporterPointSource
    variable = u
    x_coord_name = csv_reader/x3
    y_coord_name = csv_reader/y3
    z_coord_name = csv_reader/z3
    value_name = csv_reader/value3
  []
[]

[VectorPostprocessors]
  [csv_reader]
    type = CSVReader
    csv_file = point_value_file.csv
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
