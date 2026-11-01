# An interface kernel reads a neighbor property produced by a material that itself reads a property
# from another material on the neighbor block. Both neighbor-side producers must be computed.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '1 0 0'
    top_right = '2 1 0'
    block_id = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = block1
    primary_block = 0
    paired_block = 1
    new_boundary = interface
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
    type = MatDiffusion
    variable = u
    block = 0
    diffusivity = D
  []
  [diff_v]
    type = MatDiffusion
    variable = v
    block = 1
    diffusivity = D_chain
  []
[]

[InterfaceKernels]
  [interface]
    type = InterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = interface
    D = D
    D_neighbor = D_chain
  []
  # Continuity, u = v at the interface
  [continuity]
    type = InterfaceReaction
    variable = u
    neighbor_var = v
    boundary = interface
    kf = 1
    kb = 1
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  []
[]

[Materials]
  [D_block0]
    type = GenericConstantMaterial
    block = 0
    prop_names = D
    prop_values = 4
  []
  # D_chain = 1 + A = 2 on block 1. While looping over block 0, it is only consumed by the
  # interface kernel through the neighbor side, and A only through D_chain
  [A_block1]
    type = MaterialReinitTest
    block = 1
    property = A
    value = 1
  []
  [D_chain_block1]
    type = MaterialReinitTest
    block = 1
    property = D_chain
    value = 1
    coupled_property = A
  []
[]

[Postprocessors]
  # With D = 4 on block 0 and D_chain = 2 on block 1, continuity of the value and of the flux give
  # u(1) = 2/3
  [u_interface]
    type = PointValue
    variable = u
    point = '1 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
