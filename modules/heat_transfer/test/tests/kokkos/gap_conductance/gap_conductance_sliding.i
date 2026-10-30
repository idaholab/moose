# Heat conduction on two blocks separated by a gap, with nonconforming meshes, coupled across the gap
# by a mortar gap conductance constraint posed on the current configuration. The primary block slides
# along the interface by more than an element over the run, so the mortar segments pair different
# elements from step to step and part of the secondary surface loses its primary counterpart. The
# non-Kokkos and Kokkos constraints solve the same problem in separate nonlinear systems, and the
# differences between their solutions are reported.
[Mesh]
  displacements = 'disp_x disp_y'
  [left]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    nx = 3
    ny = 4
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1.2
    xmax = 2.2
    nx = 3
    ny = 7
    subdomain_ids = 1
  []
  [right_sides]
    type = RenameBoundaryGenerator
    input = right
    old_boundary = 'bottom right top left'
    new_boundary = '100 101 102 103'
  []
  [combined]
    type = CombinerGenerator
    inputs = 'left right_sides'
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = combined
    sidesets = '103'
    new_block_id = 10
    new_block_name = 'secondary_lower'
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = 'right'
    new_block_id = 20
    new_block_name = 'primary_lower'
  []
[]

[Problem]
  nl_sys_names = 'reference kokkos'
  kernel_coverage_check = false
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxKernels]
  [slide]
    type = FunctionAux
    variable = disp_y
    function = 'if(x > 1.1, 0.1 * t, 0)'
    execute_on = 'initial timestep_begin'
  []
[]

[Variables]
  [T_reference]
    block = '0 1'
    solver_sys = reference
  []
  [lambda_reference]
    block = secondary_lower
    solver_sys = reference
  []
  [T]
    block = '0 1'
    solver_sys = kokkos
  []
  [lambda]
    block = secondary_lower
    solver_sys = kokkos
  []
[]

[Kernels]
  [diff_reference]
    type = Diffusion
    variable = T_reference
  []
  [diff]
    type = KokkosDiffusion
    variable = T
  []
[]

[Constraints]
  [gap_reference]
    type = GapConductanceConstraint
    primary_boundary = right
    secondary_boundary = 103
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = lambda_reference
    secondary_variable = T_reference
    k = 0.5
    use_displaced_mesh = true
    displacements = 'disp_x disp_y'
    correct_edge_dropping = true
  []
  [gap]
    type = KokkosGapConductanceConstraint
    primary_boundary = right
    secondary_boundary = 103
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = lambda
    secondary_variable = T
    k = 0.5
    use_displaced_mesh = true
  []
[]

[BCs]
  [left_reference]
    type = DirichletBC
    variable = T_reference
    boundary = left
    value = 1
  []
  [right_reference]
    type = DirichletBC
    variable = T_reference
    boundary = 101
    value = 0
  []
  [left]
    type = KokkosDirichletBC
    variable = T
    boundary = left
    value = 1
  []
  [right]
    type = KokkosDirichletBC
    variable = T
    boundary = 101
    value = 0
  []
[]

[Postprocessors]
  [lambda_integral]
    type = ElementIntegralVariablePostprocessor
    variable = lambda
    block = secondary_lower
  []
  [T_difference]
    type = ElementL2Difference
    variable = T
    other_variable = T_reference
    block = '0 1'
  []
  [lambda_difference]
    type = ElementL2Difference
    variable = lambda
    other_variable = lambda_reference
    block = secondary_lower
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  solve_type = NEWTON
  system_names = 'reference kokkos'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Outputs]
  csv = true
[]
