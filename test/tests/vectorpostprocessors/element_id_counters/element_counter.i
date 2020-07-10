[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 1
    ymax = 1
    extra_element_integers = foo_id
  []

  [subdomain0]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    block_id = 0
    top_right = '1 1 0'
    integer_name = foo_id
  []

  [subdomain1]
    type = SubdomainBoundingBoxGenerator
    input = subdomain0
    bottom_left = '0.4 0.4 0'
    block_id = 1
    top_right = '0.9 0.9 0'
    integer_name = foo_id
  []

  [subdomain2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain1
    bottom_left = '0.1 0.1 0'
    block_id = 2
    top_right = '0.6 0.6 0'
    integer_name = foo_id
  []
[]

[VectorPostprocessors]
  [elem_counter]
    type = ElementCounterWithID
    id_name = foo_id
  []
  [elem_counter_subdomain]
    type = ElementCounterWithID
    id_name = subdomain_id
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
