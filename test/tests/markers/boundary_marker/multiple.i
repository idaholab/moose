[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [u]
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

# Mesh Marker System
[Adaptivity]
  [Markers]
    [boundary]
      type = BoundaryMarker
      next_to = 'right bottom'
      mark = refine
    []
  []
  initial_marker = boundary
  initial_steps = 3
[]

[Outputs]
  exodus = true
[]
