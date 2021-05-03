[Mesh]
  [hex_in]
    type = FileMeshGenerator
    file = gold/abtr_mesh.e
  []

  [tmg]
    type = TriangulatedMeshGenerator
    subdomain_id = 35

    # inner boundary mesh input
    inner_boundary_mesh = hex_in
    inner_boundary_name = core_out

    # outer circle boundary settings
    outer_circle_radius = 150
    outer_circle_num_segments = 50
    outer_boundary_id = 10
  []
[]
