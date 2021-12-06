[Mesh]
  [gmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = 1
    ix = 10
    dy = '0.2 0.2 0.2 0.2 0.2'
    iy = '2 2 2 2 2'
    subdomain_id = '0 1 2 3 4'
  []

  [give_subdomain_name]
    type = RenameBlockGenerator
    input = gmg
    old_block = '1 3'
    new_block = 'sub1 sub3'
  []

  [subdomain_ids]
    type = SubdomainExtraElementIDGenerator
    input = give_subdomain_name
    subdomains = '0 sub1 2 sub3'
    extra_element_id_names = 'test_id1 test_id2 test_id3'
    extra_element_ids = '
                   1 3 5 4;
                   2 9 10 11;
                   0 8 1 2
                 '
    default_extra_element_ids = '-1 0 0'
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[AuxVariables]
  [test_id1]
    family = MONOMIAL
    order = CONSTANT
  []
  [test_id2]
    family = MONOMIAL
    order = CONSTANT
  []
  [test_id3]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [test_id1]
    type = ExtraElementIDAux
    variable = test_id1
    extra_id_name = test_id1
  []
  [test_id2]
    type = ExtraElementIDAux
    variable = test_id2
    extra_id_name = test_id2
  []
  [test_id3]
    type = ExtraElementIDAux
    variable = test_id3
    extra_id_name = test_id3
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
