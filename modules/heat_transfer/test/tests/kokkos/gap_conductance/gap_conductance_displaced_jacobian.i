# The Jacobian of a mortar gap conductance constraint posed on the current configuration, with
# respect to the temperature, the Lagrange multiplier, and the displacement of the primary block.
# The secondary block stays in place and the primary block moves only normal to the vertical
# interface, so the secondary normals and the reference points on both faces do not depend on the
# displacement, and holding them fixed makes the linearization exact. The gap depends on the
# displacement through the primary points.
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
  [right_tilted]
    type = ParsedNodeTransformGenerator
    input = right_sides
    x_function = 'x'
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
  [disp_x]
    block = 0
  []
[]

[AuxVariables]
  [disp_y]
  []
[]

[ICs]
  [T]
    type = FunctionIC
    variable = T
    function = '1 + x * x + y'
  []
  [lambda]
    type = FunctionIC
    variable = lambda
    function = '1 + y'
  []
  [disp_x]
    type = FunctionIC
    variable = disp_x
    function = '-0.05 * (1 + y * y)'
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = T
  []
  [disp_x]
    type = KokkosDiffusion
    variable = disp_x
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
    use_displaced_mesh = true
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
  nl_rel_tol = 1e-12
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Outputs]
  csv = true
[]
