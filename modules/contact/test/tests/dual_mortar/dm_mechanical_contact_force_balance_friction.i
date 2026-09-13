# Extends the Newton's third law oracle in dm_mechanical_contact_force_balance.i
# to the frictional path. The parent input already slides the secondary block
# laterally against the curved interface while it is pressed closed, so
# switching to Coulomb friction adds a genuine tangential force transmission
# for the same balance check to cover: force_balance_x/force_balance_y must
# still cancel to the same precision once tangential nodal-normal-derivative
# terms are exercised, which is exactly the code path this PR's frictional
# regressions all point back to.
#
# Only the mortar_penalty formulation is exercised here (see 'tests'). The dual
# and standard Lagrange-multiplier formulations use a nonlinear complementarity
# (PDASS) function for Coulomb friction that, on this curved geometry, stagnates
# under Newton's method (DIVERGED_MAX_IT, not a divergent/NaN residual) across a
# wide scan of the c_normal/c_tangential regularization parameters; making that
# combination converge robustly would need geometry-specific tuning disproportionate
# to this check, which the penalty formulation does not need.

!include dm_mechanical_contact_force_balance.i

[Contact]
  [leftright]
    model := coulomb
    friction_coefficient = 0.1
  []
[]
