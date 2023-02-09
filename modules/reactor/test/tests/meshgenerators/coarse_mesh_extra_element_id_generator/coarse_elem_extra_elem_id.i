[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 8
    ymin = 0
    ymax = 8
    nx = 8
    ny = 8
  []
  [coarse_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 8
    ymin = 0
    ymax = 8
    nx = 3
    ny = 3
    subdomain_ids = '0 1 2
                     3 3 3
                     4 4 4'
  []
  [add_id]
    type = SubdomainExtraElementIDGenerator
    input = coarse_mesh
    subdomains = '0 1 2 3 4'
    extra_element_id_names = 'test_id'
    extra_element_ids = '4 3 2 1 0'
  []
  [coarse_id]
    type = CoarseMeshExtraElementIDGenerator
    input = gmg
    coarse_mesh = add_id
    extra_element_id_name = coarse_elem_id
    coarse_mesh_extra_element_id = test_id
    enforce_mesh_embedding = false
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
  [out]
    type = Exodus
    output_extra_element_ids = true
    show_extra_element_ids = 'coarse_elem_id'
  []
[]
