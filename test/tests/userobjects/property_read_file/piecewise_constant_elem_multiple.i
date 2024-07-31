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

[AuxVariables]
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[UserObjects]
  [reader_element]
    type = PropertyReadFile
    prop_file_names_csv = 'data_files.csv'
    read_type = 'element'
    nprop = 3  # number of columns in CSV
    execute_on = TIMESTEP_END
  []
[]

[Functions]
  [element]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'reader_element'
    read_type = 'element'
    column_number = '2'
  []
[]

[ICs]
  [element]
    type = FunctionIC
    variable = 'u'
    function = 'element'
  []
[]

[AuxKernels]
  [set_elem]
    type = FunctionAux
    variable = 'u'
    function = 'element'
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
