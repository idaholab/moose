[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
[]

[Variables]
  [u]
    block = '0'
  []
  [v]
    block = '1'
  []
[]

[UserObjects]
  [test]
    type = InterfaceDomainUserObject
    u = u
    v = v
    block = '0'
    robin_boundaries = 'left'
    interface_boundaries = 'primary0_interface'
    interface_penalty = 1e6
    nl_abs_tol = 1e-10
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
    block = 0
  []
  [force_u]
    type = BodyForce
    variable = u
    block = 0
  []
  [diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
  []
[]

[InterfaceKernels]
  [penalty_interface]
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
  []
[]

[BCs]
  [left]
    type = RobinBC
    variable = u
    boundary = 'left'
  []
  [right]
    type = RobinBC
    variable = v
    boundary = 'right'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 0
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
