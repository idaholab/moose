[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 15
    xmax = 3
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [subdomain2]
    input = subdomain1
    type = SubdomainBoundingBoxGenerator
    bottom_left = '2.0 0 0'
    block_id = 2
    top_right = '3.0 1.0 0'
  []
  [interface]
    input = subdomain2
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
[]

[Variables]
  [u]
    block = '0 1'
  []
  [v]
    block = '2'
  []
[]

[Kernels]
  [diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = '0 1'
  []
  [diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = '2'
  []
[]

[BCs]
  [bad]
    type = MatchedValueBC
    variable = u
    boundary = 'primary0_interface'
    v = v
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
