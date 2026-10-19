# Heat conduction on two blocks separated by a gap, with nonconforming meshes, coupled across the gap
# by a mortar gap conductance constraint posed on the current configuration given by a prescribed
# displacement field in three dimensions. The non-Kokkos and Kokkos constraints solve the same problem in separate
# nonlinear systems, and the differences between their solutions are reported.
[Mesh]
  displacements = 'disp_x disp_y disp_z'
  [left]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 1
    nx = 3
    ny = 4
    nz = 2
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 1.2
    xmax = 2.2
    nx = 3
    ny = 7
    nz = 3
    subdomain_ids = 1
  []
  [right_sides]
    type = RenameBoundaryGenerator
    input = right
    old_boundary = 'bottom right top left front back'
    new_boundary = '100 101 102 103 104 105'
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

# The right block rotates and both blocks shear, so that the gap width and the secondary normals vary
# along the interface
[AuxVariables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = 'if(x > 1.1, 0.1 * y - 0.05, 0)'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = '0.05 * x'
    []
  []
  [disp_z]
    [InitialCondition]
      type = FunctionIC
      function = '0.05 * y'
    []
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
    displacements = 'disp_x disp_y disp_z'
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
  type = Steady
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
