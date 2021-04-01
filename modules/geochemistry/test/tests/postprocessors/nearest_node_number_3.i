# Using NearestNodeNumber, finds the node number of the nearest node to the point in the mesh
# In this case, the point is coincident with node number 1, but then adaptivity changes this
[Mesh]
  type = GeneratedMesh
  dim = 2
  ny = 1
  nx = 4
  xmax = 8

  # For consistency with distributed mesh
  allow_renumbering = false
[]

[Adaptivity]
  marker = marker
  [Markers]
  [marker]
    type = BoxMarker
    bottom_left = '0 0 0'
    top_right = '8 1 0'
    inside = refine
    outside = do_nothing
  []
[]
[]

[UserObjects]
  [nnn_uo]
    type = NearestNodeNumberUO
    point = '2 0 0'
    execute_on = 'initial timestep_begin'
  []
[]
[Postprocessors]
  [nnn]
    type = NearestNodeNumber
    nearest_node_number_uo = nnn_uo
    execute_on = 'initial timestep_begin'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  end_time = 2
[]

[Outputs]
  csv = true
[]
