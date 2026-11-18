# Newton's third law check for mortar mechanical contact across a curved interface.
#
# The stress-divergence residual is stored in a dedicated vector tag. At a converged
# unconstrained contact node, its value is opposite the mortar contact force, so the
# tagged residual sums on the two contact boundaries must cancel.

!include dm_mechanical_contact.i

# Curve the interface while preserving the gap between the two surfaces.
curvature = 0.4

[Mesh]
  [left_curve]
    type = ParsedNodeTransformGenerator
    input = left_block_id
    x_function = 'x + ${curvature} * y * y'
  []
  [right_curve]
    type = ParsedNodeTransformGenerator
    input = right_block_id
    x_function = 'x + ${curvature} * y * y'
  []
  [combined_mesh]
    inputs := 'left_curve right_curve'
  []
[]

[Functions]
  # Keep the curved interface in contact at every output step.
  [horizontal_movement]
    y := '0.06 0.1 0.1'
  []
[]

[Problem]
  extra_tag_vectors = 'internal_force'
[]

[AuxVariables]
  [internal_force_x]
    block = '1 2'
  []
  [internal_force_y]
    block = '1 2'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    extra_vector_tags = 'internal_force'
  []
[]

[AuxKernels]
  [internal_force_x]
    type = TagVectorAux
    variable = internal_force_x
    v = disp_x
    block = '1 2'
    vector_tag = internal_force
  []
  [internal_force_y]
    type = TagVectorAux
    variable = internal_force_y
    v = disp_y
    block = '1 2'
    vector_tag = internal_force
  []
[]

[Postprocessors]
  # The parent postprocessors require a Lagrange multiplier, which the penalty case
  # does not create.
  active = 'secondary_force_x secondary_force_y primary_force_x primary_force_y '
           'force_balance_x force_balance_y'
  [secondary_force_x]
    type = NodalSum
    variable = internal_force_x
    boundary = 11
  []
  [secondary_force_y]
    type = NodalSum
    variable = internal_force_y
    boundary = 11
  []
  [primary_force_x]
    type = NodalSum
    variable = internal_force_x
    boundary = 23
  []
  [primary_force_y]
    type = NodalSum
    variable = internal_force_y
    boundary = 23
  []
  # Normalize the imbalance by the force transmitted through the secondary side.
  [force_balance_x]
    type = ParsedPostprocessor
    expression = '(sec_x + pri_x) / sqrt(sec_x^2 + sec_y^2)'
    pp_names = 'secondary_force_x secondary_force_y primary_force_x primary_force_y'
    pp_symbols = 'sec_x sec_y pri_x pri_y'
  []
  [force_balance_y]
    type = ParsedPostprocessor
    expression = '(sec_y + pri_y) / sqrt(sec_x^2 + sec_y^2)'
    pp_names = 'secondary_force_x secondary_force_y primary_force_x primary_force_y'
    pp_symbols = 'sec_x sec_y pri_x pri_y'
  []
[]

[Executioner]
  # Resolve the force imbalance well below the test tolerance.
  nl_rel_tol := 1e-12
  nl_abs_tol = 1e-8
  nl_max_its := 30
[]

[Outputs]
  file_base := mortar_mechanical_contact_force_balance
  [comp]
    show := 'force_balance_x force_balance_y'
    execute_on := 'TIMESTEP_END'
  []
[]
