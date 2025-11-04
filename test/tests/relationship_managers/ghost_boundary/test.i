[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[UserObjects]
  [all]
    type = TestGhostBoundarySideUserObject
    boundary = 'left right top bottom'
  []
  [some]
    type = TestGhostBoundarySideUserObject
    boundary = 'left right'
  []
[]

[Postprocessors]
  [num_rms]
    type = NumRelationshipManagers
    rm_type = 'geometric'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [info]
    type = Console
    system_info = 'relationship'
  []
  csv = true
[]
