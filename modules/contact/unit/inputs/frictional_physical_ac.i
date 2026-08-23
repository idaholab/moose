!include frictionless_physical.i

[Contact]
  [mortar]
    model := coulomb
    friction_coefficient = 0.4
    # Physical (derived) c_tangential, on top of the inherited physical c_normal, is this
    # fixture's default so that both physical-strategy compensation paths are exercised together.
    c_tangential_strategy = physical
    degree_one_friction_residual = true
  []
[]

[BCs]
  # Small tangential sliding to activate c_tangential
  [topx]
    function := '0.05 * t'
  []
[]
