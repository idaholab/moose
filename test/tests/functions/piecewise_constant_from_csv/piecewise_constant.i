[Mesh]
  allow_renumbering = false
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    ix = '2 1 1'
    iy = '1 3'
    subdomain_id = '0 1 1
                    2 2 2'
  []
[]

[Variables]
  [u]
  []
[]

[UserObjects]
  [reader_element]
    type = PropertyReadFile
    prop_file_name = 'data_element.csv'
    read_type = 'element'
    nprop = 3  # number of columns in CSV
  []
  [reader_node]
    type = PropertyReadFile
    prop_file_name = 'data_node.csv'
    read_type = 'node'
    nprop = 3  # number of columns in CSV
  []
  [reader_nearest]
    type = PropertyReadFile
    prop_file_name = 'data_nearest.csv'
    read_type = 'voronoi'
    nprop = 3
    nvoronoi = 3
  []
  [reader_block]
    type = PropertyReadFile
    prop_file_name = 'data_nearest.csv'
    read_type = 'block'
    nprop = 3
    nblock = 3
  []
[]

[Functions]
  [element]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'reader_element'
    read_type = 'element'
    column_number = '2'
  []
  [node]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'reader_node'
    read_type = 'node'
    column_number = '2'
  []
  [nearest]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'reader_nearest'
    read_type = 'voronoi'
    column_number = '2'
  []
  [block]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'reader_block'
    read_type = 'block'
    column_number = '2'
  []
[]

[ICs]
  active = 'element'
  [element]
    type = FunctionIC
    variable = 'u'
    function = 'element'
  []
  [node]
    type = FunctionIC
    variable = 'u'
    function = 'node'
  []
  [nearest]
    type = FunctionIC
    variable = 'u'
    function = 'nearest'
  []
  [block]
    type = FunctionIC
    variable = 'u'
    function = 'block'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [unity]
    type = DirichletBC
    variable = u
    boundary = 'left bottom'
    value = 1
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  end_time = 0.1
[]

[Outputs]
  exodus = true
[]
