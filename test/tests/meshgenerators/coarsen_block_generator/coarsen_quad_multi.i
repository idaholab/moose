[Mesh]
  allow_renumbering = false
  [eg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '2 1 1'
    dy = '2 3'
    ix = '2 1 1'
    iy = '2 3'
    subdomain_id = '0 1 1
                    1 2 0'
  []

  [refine]
    type = RefineBlockGenerator
    input = eg
    block = '0 1 2'
    refinement = '1 1 1'
    enable_neighbor_refinement = false
    output = true
  []

  # Go back to what we had
  [coarsen]
    type = CoarsenBlockGenerator
    input = refine
    block = '0 1 2'
    coarsening = '1 1 1'
    # This was found by looking at the output of refine
    # careful, has transitions we dont support!
    start_elem_id = 56
  []
[]

[Outputs]
  exodus = true
[]
