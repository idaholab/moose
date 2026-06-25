# Regression test for #30209.
# This used to fail in debug mode with:
#   dof_values.size() == _dof_indices.size()
# in MooseVariableDataBase::insert().
#
# The trigger is the mortar nodal WeightedGapAux path after node-face
# reinitialization with minimum_projection_angle = 50.

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Mesh]
  [input_file]
    type = FileMeshGenerator
    file = iron.e
  []

  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = input_file
    sidesets = '10'
    new_block_id = 3
    new_block_name = 'fric_primary_subdomain'
  []

  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = '20'
    new_block_id = 4
    new_block_name = 'fric_secondary_subdomain'
  []

  patch_update_strategy = always
[]

[AuxVariables]
  [mortar_gap]
  []
[]

[AuxKernels]
  [gap]
    type = WeightedGapAux
    variable = mortar_gap

    primary_boundary = 10
    primary_subdomain = 3
    secondary_boundary = 20
    secondary_subdomain = 4

    correct_edge_dropping = true
    minimum_projection_angle = 50
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]
