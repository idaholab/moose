# Extend the curved-interface force-balance check to Coulomb friction. The penalty
# formulation avoids the geometry-specific PDASS tuning needed by the Lagrange-
# multiplier formulations for this input.

!include mortar_mechanical_contact_force_balance.i

[Contact]
  [leftright]
    model := coulomb
    friction_coefficient = 0.1
  []
[]
