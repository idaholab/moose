[Mesh]
  [msh]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.25 0.25 0.25 0.25'
    dy = '0.25 0.25 0.25 0.25'
    dz = '0.25 0.25 0.25 0.25'
    subdomain_id = '0 1 2 3 4 5 6 7 8 9 10 11 12
       13 14 15 16 17 18 19 20 21 22 23 24 25
       26 27 28 29 30 31 32 33 34 35 36 37 38
       39 40 41 42 43 44 45 46 47 48 49 50 51
       52 53 54 55 56 57 58 59 60 61 62 63'
  []
  [split]
    input = msh
    type = BreakMeshByBlockGenerator
    surrounding_blocks = '16 17 18 19 20 21 22 23 24 25
       26 27 28 29 30 31 32 34 35 36 37 38
       39 40 41 42 43 44 45 46 47'
    add_transition_interface = true
    add_interface_on_two_sides = false
    split_interface = false
  []
[]
