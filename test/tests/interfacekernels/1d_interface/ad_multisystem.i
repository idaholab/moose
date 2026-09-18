# Two AD variables coupled by penalty interface kernels, each living on its own
# nonlinear system. This exercises the multi-system support in AD interface
# kernels: an interface kernel must only contribute to the system of its
# 'variable', never to the 'neighbor_var' rows that belong to a different system.
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

[Problem]
  nl_sys_names = 'u_sys v_sys'
  # Each system holds a single block-restricted variable, so the per-system
  # coverage check (which expects every mesh block covered) does not apply.
  kernel_coverage_check = false
  # Guards against an interface kernel inserting into the wrong system's matrix:
  # the neighbor rows/columns live in a different system and are not part of the
  # current system's sparsity pattern.
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [u]
    solver_sys = 'u_sys'
    block = '0'
  []
  [v]
    solver_sys = 'v_sys'
    block = '1'
  []
[]

[Kernels]
  [diff_u]
    type = ADDiffusion
    variable = u
    block = 0
  []
  [diff_v]
    type = ADDiffusion
    variable = v
    block = 1
  []
[]

[InterfaceKernels]
  # One interface kernel per direction so that both systems receive an interface
  # contribution and both Jacobians are exercised.
  [interface_u]
    type = ADPenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e3
  []
  [interface_v]
    type = ADPenaltyInterfaceDiffusion
    variable = v
    neighbor_var = u
    boundary = primary0_interface
    penalty = 1e3
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [right]
    type = ADDirichletBC
    variable = v
    boundary = 'right'
    value = 0
  []
[]

[Preconditioning]
  [u]
    type = SMP
    nl_sys = u_sys
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
  [v]
    type = SMP
    nl_sys = v_sys
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
[]

[Executioner]
  type = SteadySolve2
  solve_type = 'NEWTON'
  first_nl_sys_to_solve = 'u_sys'
  second_nl_sys_to_solve = 'v_sys'
[]

[Outputs]
  exodus = true
[]
