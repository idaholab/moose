[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1 1'
    ix = '2 2 2 2'
    dy = 1
    iy = 2
    subdomain_id = '0 1 2 3'
  []
  # One boundary joining the block pairs 0 -> 1, 2 -> 1 and 2 -> 3
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = gen
    new_boundary = interface
    primary_block = '0 2'
    paired_block = '1 3'
  []
[]

[Variables]
  [u]
    block = 0
  []
  [v]
    block = 3
  []
  [w]
    block = '1 2'
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [diff_w]
    type = Diffusion
    variable = w
  []
[]

[InterfaceKernels]
  [penalty]
    type = PenaltyInterfaceDiffusion
    # 'variable' is on a primary block and 'neighbor_var' on a secondary block, but no face of
    # the boundary joins block 0 to block 3
    variable = u
    neighbor_var = v
    penalty = 1
    boundary = interface
  []
[]

[Executioner]
  type = Steady
[]
