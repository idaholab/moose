[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 1
    ymax = 1
    extra_element_integers = test_id
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

  [copy_test_id]
    type = ExtraElementIDCopyGenerator
    input = another_subdomains
    source_extra_element_id = test_id
    target_extra_element_ids = 'test_id1 test_id2'
  []

  [copy_test_id1]
    type = ExtraElementIDCopyGenerator
    input = copy_test_id
    source_extra_element_id = subdomain_id
    target_extra_element_ids = 'test_id3'
  []

  [copy_test_id2]
    type = ExtraElementIDCopyGenerator
    input = copy_test_id1
    source_extra_element_id = element_id
    target_extra_element_ids = 'test_id4'
  []
  # element id could be renumbered with distributed mesh
  # causing exodiff on test_id4 variable, thus we turn off
  # this flag, normal calculations are fine with element IDs
  # being renumbered.
  allow_renumbering = false
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = Exodus
    output_extra_element_ids = true
    show_extra_element_ids = 'test_id test_id1 test_id2 test_id3 test_id4'
  []
[]
