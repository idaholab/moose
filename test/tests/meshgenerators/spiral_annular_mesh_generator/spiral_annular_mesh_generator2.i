[Mesh]
  [./samg]
    type = SpiralAnnularMeshGenerator
    input = eg
    use_tri6 = false
    inner_radius = 2
    nodes_per_ring = 12
    outer_radius = 7
    num_rings = 5
  []

  [./eg2]
    type = ElementGenerator
    input = samg
    nodal_positions = '0 0 0
                       -5 0 0
                       0 5 0'
    element_connectivity = '0 1 2'
    elem_type = TRI3
  []
[]

[Outputs]
  exodus = true
[]
