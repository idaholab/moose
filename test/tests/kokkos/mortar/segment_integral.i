# Two blocks meeting along x = 1 with nonconforming meshes on either side, so that the mortar
# segments split the secondary and primary faces at different points.
[Mesh]
  [left]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    nx = 2
    ny = 3
    elem_type = QUAD4
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1
    xmax = 2
    nx = 2
    ny = 5
    elem_type = QUAD4
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
  [dummy]
    block = '0 1'
  []
[]

[AuxVariables]
  [u]
    block = 'secondary_lower primary_lower'
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = '1 + y + y^2'
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = dummy
  []
[]

[Constraints]
  # Builds the mortar interface; the constraint itself only ties dummy across it
  [mortar]
    type = EqualValueConstraint
    primary_boundary = 103
    secondary_boundary = right
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = lambda
    secondary_variable = dummy
  []
[]

[Variables]
  [lambda]
    block = secondary_lower
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    variable = dummy
    boundary = left
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    variable = dummy
    boundary = 101
    value = 1
  []
[]

[Postprocessors]
  [segment_integral]
    type = KokkosMortarSegmentIntegral
    primary_boundary = 103
    secondary_boundary = right
    primary_subdomain = primary_lower
    secondary_subdomain = secondary_lower
    variable = u
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
[]

[Outputs]
  csv = true
[]
