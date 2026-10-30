# Heat conduction on two blocks separated by a gap whose width varies over the interface, with
# nonconforming meshes, coupled across the gap by a mortar gap conductance constraint in three
# dimensions
[Mesh]
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
  # Tilt the right block so that the gap widens from bottom to top
  [right_tilted]
    type = ParsedNodeTransformGenerator
    input = right_sides
    x_function = 'x + 0.2 * y + 0.1 * z'
    y_function = 'y'
    z_function = 'z'
  []
  [combined]
    type = CombinerGenerator
    inputs = 'left right_tilted'
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

[Variables]
  [T]
    block = '0 1'
  []
  [lambda]
    block = secondary_lower
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = T
  []
[]

[Constraints]
  [gap]
    type = KokkosGapConductanceConstraint
    primary_boundary = right
    secondary_boundary = 103
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = lambda
    secondary_variable = T
    k = 0.5
  []
[]

[BCs]
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
  [T_integral]
    type = ElementIntegralVariablePostprocessor
    variable = T
    block = '0 1'
  []
  [lambda_integral]
    type = ElementIntegralVariablePostprocessor
    variable = lambda
    block = secondary_lower
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Outputs]
  csv = true
[]
