[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    extra_element_integers = 'id1 id2'
  []
  [id1_bottom_left]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    top_right = '3 3 0'
    block_id = 0
    location = inside
    integer_name = id1
  []
  [id1]
    type = SubdomainBoundingBoxGenerator
    input = id1_bottom_left
    bottom_left = '0 0 0'
    top_right = '3 3 0'
    block_id = 2
    location = outside
    integer_name = id1
  []
  [id2_bottom_left]
    type = SubdomainBoundingBoxGenerator
    input = id1
    bottom_left = '0 0 0'
    top_right = '5 5 0'
    block_id = 1
    location = inside
    integer_name = id2
  []
  [id2]
    type = SubdomainBoundingBoxGenerator
    input = id2_bottom_left
    bottom_left = '0 0 0'
    top_right = '5 5 0'
    block_id = 3
    location = outside
    integer_name = id2
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters]
  [id_map]
    type = ElementIDTest
    id_name1 = id1
    id_name2 = id2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  json = true
[]
