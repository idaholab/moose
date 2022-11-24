[Mesh]
  [./msh]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 3
    nz = 4
    xmin = -2.5
    xmax = 2.5
    ymin = -2
    ymax = 2
    zmin = -1.5
    zmax = 1.5
    dim = 3
  [../]
  [Partitioner]
    type = GridPartitioner
    nx = 1
    ny = 3
    nz = 4
  []
  [./subdomain_1]
    type = SubdomainBoundingBoxGenerator
    input = msh
    bottom_left = '-2.5 -2 -1'
    top_right = '2.5 0 0.5'
    block_id = 1
  []
  [./subdomain_2]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_1
    bottom_left = '-2.5 0 -1'
    top_right = '2.5 2 0.5'
    block_id = 2
  []
  [./subdomain_3]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_2
    bottom_left = '-2.5 -2 0.5'
    top_right = '1.25 2 1.5'
    block_id = 3
  []
  [./subdomain_4]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_3
    bottom_left = '1.25 -2 0.5'
    top_right = '5 2 1.5'
    block_id = 4
  []
  [./subdomain_5]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_4
    bottom_left = '-2.5 -2 -1.5'
    top_right = '1.25 2 -1'
    block_id = 3
  []
  [./subdomain_6]
    type = SubdomainBoundingBoxGenerator
    input = subdomain_5
    bottom_left = '1.25 -2 -1.5'
    top_right = '2.5 2 -1'
    block_id = 4
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = subdomain_6
  []
[]

[Debug]
  output_process_domains = true
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
