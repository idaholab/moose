[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 1
    ymax = 1
    subdomain_ids = '0 0 0 0 0 0 0 0 0 0
                     0 1 0 0 0 0 0 0 0 0
                     0 0 2 0 0 0 0 0 0 0
                     0 0 0 3 0 0 0 0 0 0
                     0 0 0 0 4 0 0 0 0 0
                     0 0 0 0 0 5 0 0 0 0
                     0 0 0 0 0 0 6 0 0 0
                     0 0 0 0 0 0 0 7 0 0
                     0 0 0 0 0 0 0 0 8 0
                     0 0 0 0 0 0 0 0 0 9'
    extra_element_integers = 'pin_id temp_id'
  []
  [pinid_1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    top_right = '0.5 0.5 0'
    block_id = 1
    location = INSIDE
    integer_name = pin_id
  []
  [pinid_2]
    type = SubdomainBoundingBoxGenerator
    input = pinid_1
    bottom_left = '0.5 0 0'
    top_right = '1 0.5 0'
    block_id = 2
    location = INSIDE
    integer_name = pin_id
  []
  [pinid_3]
    type = SubdomainBoundingBoxGenerator
    input = pinid_2
    bottom_left = '0 0.5 0'
    top_right = '0.5 1 0'
    block_id = 3
    location = INSIDE
    integer_name = pin_id
  []
  [pinid_4]
    type = SubdomainBoundingBoxGenerator
    input = pinid_3
    bottom_left = '0.5 0.5 0'
    top_right = '1 1 0'
    block_id = 4
    location = INSIDE
    integer_name = pin_id
  []
  [tempid_1]
    type = SubdomainBoundingBoxGenerator
    input = pinid_4
    bottom_left = '0 0 0'
    top_right = '0.5 0.5 0'
    block_id = 1
    location = INSIDE
    integer_name = temp_id
  []
  [tempid_2]
    type = SubdomainBoundingBoxGenerator
    input = tempid_1
    bottom_left = '0.5 0 0'
    top_right = '1 0.5 0'
    block_id = 2
    location = INSIDE
    integer_name = temp_id
  []
  [tempid_3]
    type = SubdomainBoundingBoxGenerator
    input = tempid_2
    bottom_left = '0 0.5 0'
    top_right = '0.5 1 0'
    block_id = 3
    location = INSIDE
    integer_name = temp_id
  []
  [tempid_4]
    type = SubdomainBoundingBoxGenerator
    input = tempid_3
    bottom_left = '0.5 0.5 0'
    top_right = '1 1 0'
    block_id = 4
    location = INSIDE
    integer_name = temp_id
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Debug]
  show_actions = true
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = true
  []
[]
