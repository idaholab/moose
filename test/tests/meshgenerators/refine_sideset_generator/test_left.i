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
  [refine]
    type = RefineSidesetGenerator
    input = eg
    boundaries = 'left'
    refinement = '3'
    boundary_side = 'primary'
    enable_neighbor_refinement = false
  []
[]

[Outputs]
  exodus = true
[]
