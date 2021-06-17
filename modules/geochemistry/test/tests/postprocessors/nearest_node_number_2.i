# Using NearestNodeNumber, finds the node number of the nearest node to the point in the mesh
# In this case, the mesh has two disjoint parts and the point is equidistant from the two parts, so the closest node with the smallest ID is chosen.
# This input file is run multiple times:
#  - 1 thread and 1 process
#  - 2 threads and 1 process
#  - 1 thread and 2 processes
#  - 2 threads and 2 processes
# Each time should give the same result
[Mesh]
  [left]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
    xmin = 0
    xmax = 8
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
    xmin = 12
    xmax = 20
  []
  [combiner]
    type = CombinerGenerator
    inputs = 'left right'
  []

  # For consistency with distributed mesh
  allow_renumbering = false
[]

[UserObjects]
  [nnn_uo]
    type = NearestNodeNumberUO
    point = '10 0 0'
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
