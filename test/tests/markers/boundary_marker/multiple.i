###########################################################
# This is a test of the Mesh Marker System. It marks
# elements with flags indicating whether they should be
# refined, coarsened, or left alone. This system
# has the ability to use the Mesh Indicator System.
#
# @Requirement F2.50
###########################################################

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
