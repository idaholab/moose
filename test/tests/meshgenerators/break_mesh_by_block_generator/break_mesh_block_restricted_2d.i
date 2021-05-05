[Mesh]
  [msh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.25 0.25 0.25 0.25'
    dy = '0.25 0.25 0.25 0.25'
    subdomain_id = '0 1 2 3 4 5 6 7 8 9 10 11 12
       13 14 15'
  []
  [split]
    input = msh
    type = BreakMeshByBlockGenerator
    block = '5 6 9 10'
    add_transition_interface = false
    split_interface = true
  []
[]
