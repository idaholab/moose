[Problem]
  solve = false
[]

[Mesh]
  add_subdomain_ids = 3
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.25 1 1'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    bottom_left = '0.25 0 0'
    top_right = '1 1 1'
  []
  [boundary]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'right'
    new_boundary = 'moving_boundary'
    primary_block = '1'
    paired_block = '2'
  []
[]

[AuxVariables]
  [dummy]
  []
[]

[MeshModifiers]
  [subdomain_change]
    type = TimedSubdomainModifier
    times = '0.3 0.6'
    blocks_from = '1 3'
    blocks_to = '3 1'
    moving_boundaries = 'moving_boundary'
    moving_boundary_subdomain_pairs = '1 2'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 0.3
  num_steps = 3
[]

[Outputs]
  exodus = true
[]

