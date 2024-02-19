[Mesh]
  allow_renumbering = false
  # use these two to re-generate input.e
  [eg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '2 1 1'
    dy = '2 3'
    dz = '2 3'
    ix = '2 1 1'
    iy = '2 3'
    iz = '3 2'
    subdomain_id = '0 1 1
                    1 2 0
                    0 1 1
                    1 2 0'
  []
  [add_internal]
    type = SideSetsBetweenSubdomainsGenerator
    input = eg
    paired_block = 1
    primary_block = 2
    new_boundary = middle
  []
  [refine]
    type = RefineBlockGenerator
    input = add_internal
    block = '0 1 2'
    refinement = '1 1 1'
    enable_neighbor_refinement = false
    output = true
  []

  final_generator = 'diag'
  [input]
    type = FileMeshGenerator
    file = 'single_nonuniform_hex.e'
  []

  # Go back to what we had
  [coarsen]
    type = CoarsenBlockGenerator
    input = input
    block = '0 1 2'
    coarsening = '1 1 1'
    # This was found by looking at the output of refine
    # careful, has transitions we dont support!
    starting_point = '2.75 0.75 0'
    maximum_volume_ratio = 4
    verbose = true
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = coarsen
    examine_element_overlap = ERROR
    search_for_adaptivity_nonconformality = WARNING
    examine_non_conformality = WARNING
  []
[]

[Outputs]
  exodus = true
[]
