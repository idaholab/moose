[Mesh]
  [gmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '2 1'
    ix = '1 2'
    iy = '3 2'
    subdomain_id = '0 1 2 3'
  []
  [add_eeid]
    type = PlaneIDMeshGenerator
    input = 'gmg'
    plane_coordinates = '-2 0 1 3'
    num_ids_per_plane = ' 1 2 1'
    plane_axis = 'y'
    id_name = 'test_id1'
    tolerance = 1
  []
  [subdomains]
    type = ParsedSubdomainMeshGenerator
    input = add_eeid
    # Only elements on subdomain 2 and 3 should match this
    combinatorial_geometry = 'test_id1 > 2'
    extra_element_id_names = 'test_id1'
    block_id = 4
  []
[]

[Outputs]
  exodus = true
[]
