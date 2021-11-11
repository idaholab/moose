[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 1
    ymax = 1
    extra_element_integers = test_id
  []

  [subdomains]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    block_id = 1
    top_right = '0.9 0.9 0'
    integer_name = test_id
  []

  [another_subdomains]
    type = SubdomainBoundingBoxGenerator
    input = subdomains
    bottom_left = '0 0 0'
    block_id = 2
    top_right = '0.9 0.9 0'
    location = OUTSIDE
    integer_name = test_id
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[AuxVariables]
  [test_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [test_id]
    type = ExtraElementIDAux
    variable = test_id
    extra_id_name = test_id
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
