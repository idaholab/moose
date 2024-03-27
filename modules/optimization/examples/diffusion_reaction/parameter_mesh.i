[Mesh]
  [gmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5'
    dy = '0.5 0.5'
    ix = '1 1'
    iy = '1 1'
    subdomain_id = '0 0
                    0 1'
  []
  parallel_type = REPLICATED
[]

[Adaptivity]
  initial_steps = 2
  initial_marker = box
  [Markers/box]
    type = BoxMarker
    bottom_left = '0.75 0.75 0'
    top_right = '1 1 0'
    inside = REFINE
    outside = DO_NOTHING
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = TIMESTEP_END
[]
