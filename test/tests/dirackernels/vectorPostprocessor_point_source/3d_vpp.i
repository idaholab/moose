[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  [./point_source]
    type = VectorPostprocessorPointSource
    variable = u
    vector_postprocessor = csv_reader
    x_coord_name = x3
    y_coord_name = y3
    z_coord_name = z3
    value_name = value3
  [../]
[]

[VectorPostprocessors]
  [./csv_reader]
    type = CSVReader
    csv_file = point_value_file.csv
    execute_on = initial
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
