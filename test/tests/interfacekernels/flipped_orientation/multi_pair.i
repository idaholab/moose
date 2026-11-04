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
    block = '0 2'
  []
  [v]
    block = 1
  []
  [w]
    block = 3
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
    # Valid on the pairs 0 -> 1 and 2 -> 1, but 'neighbor_var' is not defined on block 3, so
    # the kernel would be skipped on the faces of the pair 2 -> 3
    variable = u
    neighbor_var = v
    penalty = 1
    boundary = interface
  []
[]

[Executioner]
  type = Steady
[]
