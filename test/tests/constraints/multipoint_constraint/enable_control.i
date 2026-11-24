# Two unconnected halves of a unit square, with u = 0 on the left side and u = 1 on the right
# side. The exact constraints tie each node of the right half on the interface to the coincident
# node of the left half, so u is linear in x and continuous: both interface values are 0.375 and
# tie_error is 0. The Control disables the constraint for the second time step. The halves then
# solve separately, the left half gives u = 0 and the right half gives u = 1, so tie_error is 1.
# The constraint is enabled again for the third time step, and tie_error returns to 0.

[Mesh]
  [left_half]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.375
    nx = 3
    ny = 4
    boundary_name_prefix = left_half
  []
  [right_half]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.375
    xmax = 1
    nx = 5
    ny = 4
    boundary_name_prefix = right_half
    boundary_id_offset = 10
  []
  # The node sets are made before the halves are combined, because the coordinate names two nodes
  # afterwards
  [primary_mid]
    type = ExtraNodesetGenerator
    input = left_half
    coord = '0.375 0.5'
    new_boundary = primary_mid
  []
  [secondary_mid]
    type = ExtraNodesetGenerator
    input = right_half
    coord = '0.375 0.5'
    new_boundary = secondary_mid
  []
  [split]
    type = CombinerGenerator
    inputs = 'primary_mid secondary_mid'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left_half_left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right_half_right
    value = 1
  []
[]

[Constraints]
  [tie]
    type = TestMultiPointConstraint
    secondary_variable = u
    secondary_boundary = right_half_left
    primary_boundary = left_half_right
  []
[]

[Controls]
  [release]
    type = TimePeriod
    disable_objects = 'Constraints::tie'
    start_time = 1.5
    end_time = 2.5
    execute_on = 'initial timestep_begin'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 3
  dt = 1
[]

[Postprocessors]
  [u_secondary]
    type = AverageNodalVariableValue
    variable = u
    boundary = secondary_mid
  []
  [u_primary]
    type = AverageNodalVariableValue
    variable = u
    boundary = primary_mid
  []
  [tie_error]
    type = DifferencePostprocessor
    value1 = u_secondary
    value2 = u_primary
  []
[]

[Outputs]
  csv = true
[]
