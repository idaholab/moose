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

  [id0]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    block_id = 0
    top_right = '1 1 0'
    integer_name = foo_id
  []

  [id1]
    type = SubdomainBoundingBoxGenerator
    input = id0
    bottom_left = '0.4 0.4 0'
    block_id = 1
    top_right = '0.9 0.9 0'
    integer_name = foo_id
  []

  [id2]
    type = SubdomainBoundingBoxGenerator
    input = id1
    bottom_left = '0.1 0.1 0'
    block_id = 2
    top_right = '0.6 0.6 0'
    integer_name = foo_id
  []

  [subdomain]
    type = SubdomainBoundingBoxGenerator
    input = id2
    bottom_left = '0 0.6 0'
    block_id = 1
    top_right = '1 1 0'
  []
[]

[VectorPostprocessors]
  [elem_counter]
    type = InternalSideElementCounterWithID
    id_name = foo_id
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
