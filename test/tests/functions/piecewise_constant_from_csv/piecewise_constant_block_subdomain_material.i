# Regression test: a constant_on = SUBDOMAIN material reading block-sorted CSV data through a
# PiecewiseConstantFromCSV function. Such a material is computed during subdomainSetup, before the
# current element's quadrature points are reinitialized, so the point handed to the function is a
# stale/invalid q-point. Previously the function point-located that point and errored ("No element
# located ..."); it must instead read the value for the subdomain currently being processed. The
# three subdomains (0, 1, 2) must pick up column 3 of data_nearest.csv: 0.1, 4, 7 respectively.
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

[UserObjects]
  [reader_block]
    type = PropertyReadFile
    prop_file_name = 'data_nearest.csv'
    read_type = 'block'
    nprop = 4 # number of columns in CSV
    nblock = 3 # number of rows that are considered
  []
[]

[Functions]
  [block]
    type = PiecewiseConstantFromCSV
    read_prop_user_object = 'reader_block'
    read_type = 'block'
    # 0-based indexing
    column_number = '3'
  []
[]

[Materials]
  [block_mat]
    type = GenericFunctionMaterial
    prop_names = 'block_prop'
    prop_values = 'block'
    # The path under test: evaluate the function during subdomainSetup, not per quadrature point.
    constant_on = SUBDOMAIN
  []
[]

[AuxVariables]
  [block_aux]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [block_aux]
    type = MaterialRealAux
    variable = block_aux
    property = block_prop
    execute_on = 'INITIAL TIMESTEP_END'
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
