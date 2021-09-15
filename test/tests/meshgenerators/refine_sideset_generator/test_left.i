[Mesh]
  [eg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '2 1 1'
    dy = '2 3'
    dz = '0.4 0.5 0.6 0.7'
    ix = '2 1 1'
    iy = '2 3'
    iz = '1 1 1 1'
    subdomain_id = '0 1 1 1
                    1 2 0 1
                    0 1 1 1
                    2 2 2 2
                    3 3 1 3
                    1 1 1 1'
  []
  [sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = eg
    primary_block = 1
    paired_block = 2
    new_boundary = sideset_1
  []
  [refine]
    type = RefineSidesetGenerator
    input = sideset
    boundaries = 'sideset_1'
    refinement = '2'
    boundary_side = 'secondary'
    enable_neighbor_refinement = false
  []
[]

[Outputs]
  exodus = true
[]
