[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = square
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = left
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[VectorPostprocessors]
  [source]
    type = CSVReader
    csv_file = point_value_file.csv
  []
[]

[DiracKernels]
  [point_source]
    type = ReporterPointSource
    variable = u
    block = 1
    value_name = source/value
    x_coord_name = source/x
    y_coord_name = source/y
    z_coord_name = source/z
    # The VPP contains the following information
    # x,y,z,value
    # 0.25,0.25,0.0,1
    # 0.50,0.50,0.0,2
    # 0.75,0.75,0.0,3
    # The first point is in block 1.
    # The second point is on the interface between blocks 1 and 2.
    # The third point is in block 2.
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

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
