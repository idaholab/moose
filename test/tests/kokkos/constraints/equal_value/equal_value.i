# Diffusion on two blocks meeting along x = 1 with nonconforming meshes, whose solutions are tied
# across the interface by a mortar equal value constraint
[Mesh]
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
    xmin = 1
    xmax = 2
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
    sidesets = 'right'
    new_block_id = 10
    new_block_name = 'secondary_lower'
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = '103'
    new_block_id = 20
    new_block_name = 'primary_lower'
  []
[]

[Variables]
  [u]
    block = '0 1'
  []
  [lambda]
    block = secondary_lower
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [source]
    type = KokkosBodyForce
    variable = u
    value = 1
  []
[]

[Constraints]
  [mortar]
    type = KokkosEqualValueConstraint
    primary_boundary = 103
    secondary_boundary = right
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = lambda
    secondary_variable = u
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = 101
    value = 1
  []
[]

[Postprocessors]
  [u_integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
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
