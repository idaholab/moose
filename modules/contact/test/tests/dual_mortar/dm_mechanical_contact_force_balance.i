# Newton's third law check for mortar mechanical contact across a curved interface.
#
# This extends the two-block mortar regression in dm_mechanical_contact.i with a
# curved contact interface and an independent force-balance oracle on the two
# contact sidesets.
#
# Oracle: 'save_in' collects only the stress-divergence (volumetric) residual
# contribution, because the mortar constraints do not participate in it. At a
# converged node that carries no other residual object, f_internal + f_contact = 0,
# so summing the saved residual over one contact sideset returns minus the total
# mortar contact force transmitted through that side. Newton's third law then
# requires the two sideset sums to cancel. The identity is exact here because
# sidesets 11 and 23 carry no boundary condition: Dirichlet data is applied only
# on the far faces 13 and 21.
#
# The interface has to be curved. A flat interface balances trivially, because
# every nodal normal on it is identical.

!include dm_mechanical_contact.i

# Amplitude of the x -> x + curvature * y^2 shift applied to both blocks below.
# 0.4 turns the secondary surface normal by roughly 3 to 6 degrees per element
# across the contact patch: mild enough to solve robustly, and enough curvature
# for a force imbalance to sit far above the nonlinear tolerance set below.
curvature = 0.4

[Mesh]
  # Shift both blocks by the same function of y. This curves the interface while
  # leaving the horizontal separation of the two contact surfaces unchanged.
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
  # Start with the 0.05 initial gap already closed so that contact is active at
  # every output step. The normalized balance ratios below are undefined while
  # the two bodies are separated.
  [horizontal_movement]
    y := '0.06 0.1 0.1'
  []
[]

[AuxVariables]
  [saved_x]
  []
  [saved_y]
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    save_in = 'saved_x saved_y'
  []
[]

[Postprocessors]
  # The parent's postprocessors all read the Lagrange multiplier variable, which
  # does not exist for the penalty formulation this input is also run with.
  active = 'secondary_force_x secondary_force_y primary_force_x primary_force_y '
           'force_balance_x force_balance_y'
  [secondary_force_x]
    type = NodalSum
    variable = saved_x
    boundary = 11
  []
  [secondary_force_y]
    type = NodalSum
    variable = saved_y
    boundary = 11
  []
  [primary_force_x]
    type = NodalSum
    variable = saved_x
    boundary = 23
  []
  [primary_force_y]
    type = NodalSum
    variable = saved_y
    boundary = 23
  []
  # Imbalance normalized by the magnitude of the force transmitted through the
  # secondary side, so that the expected value is zero independently of the load.
  # If the interface were to separate, both sums collapse to round-off and the
  # ratio becomes ill-conditioned rather than small, giving O(1) values that fail
  # loudly. That is deliberate: a vacuous run must not report balance.
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
  # The oracle infers the contact force from nodal equilibrium, so the nonlinear
  # residual has to be driven well below the imbalance being measured. The
  # relative tolerance is what actually governs here: measured final residuals
  # are 3.6e-8, 7.0e-9, 1.2e-8, 1.2e-8 and 2.0e-8 over the five steps, so
  # nl_abs_tol is only reached on the second step and nl_rel_tol decides the
  # other four. Together they leave the balance ratios resolved to roughly 1e-12.
  nl_rel_tol := 1e-12
  nl_abs_tol = 1e-8
  nl_max_its := 30
[]

[Outputs]
  file_base := dm_mechanical_contact_force_balance
  [comp]
    # Only the dimensionless balance ratios are verified. The contact force
    # magnitudes are solution quantities that legitimately change whenever the
    # mortar contact force is reformulated.
    show := 'force_balance_x force_balance_y'
    execute_on := 'TIMESTEP_END'
  []
[]
