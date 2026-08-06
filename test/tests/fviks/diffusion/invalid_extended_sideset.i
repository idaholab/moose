[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    xmax = 3
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '1 0 0'
    top_right = '2 1 0'
    block_id = 1
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = block1
    bottom_left = '2 0 0'
    top_right = '3 1 0'
    block_id = 2
  []
  [interface01]
    type = SideSetsBetweenSubdomainsGenerator
    input = block2
    primary_block = 0
    paired_block = 1
    new_boundary = interface01
  []
  [interface12]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface01
    primary_block = 1
    paired_block = 2
    new_boundary = interface12
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    block = 0
  []
  [v]
    type = MooseVariableFVReal
    block = 1
  []
[]

[FVInterfaceKernels]
  [interface]
    type = FVDiffusionInterface
    variable1 = u
    variable2 = v
    subdomain1 = 0
    subdomain2 = 1
    boundary = 'interface01 interface12'
    coeff1 = 1
    coeff2 = 1
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]
