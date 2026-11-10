[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 3
    ymax = 1
    nx = 3
    ny = 1
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '1 1 0'
  []
  [middle]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    bottom_left = '1 0 0'
    top_right = '2 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'middle'
    block_id = 3
    bottom_left = '2 0 0'
    top_right = '3 1 0'
  []
[]

[AuxVariables]
  [dummy]
    type = MooseVariableFVReal
  []
[]

# blocks 2 and 3 both change to block 1, at times 0 and 1 respectively
[MeshModifiers]
  [block_change]
    type = TimedSubdomainModifier
    times = '0 1'
    blocks_from = '2 3'
    blocks_to = '1 1'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 2
[]

[Outputs]
  exodus = true
[]
