[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    ix = '2 2'
    dy = 1
    iy = 2
    subdomain_id = '0 1'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = gen
    new_boundary = interface
    primary_block = 0
    paired_block = 1
  []
[]

[Variables]
  [u]
    block = 0
  []
  [v]
    block = 1
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
[]

[InterfaceKernels]
  [penalty]
    type = PenaltyInterfaceDiffusion
    # Flipped relative to the sideset, whose primary side is block 0
    variable = v
    neighbor_var = u
    penalty = 1
    boundary = interface
  []
[]

[Executioner]
  type = Steady
[]
