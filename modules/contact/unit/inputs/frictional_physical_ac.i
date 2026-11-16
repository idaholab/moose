!include frictionless_physical.i

[Contact]
  [mortar]
    model := coulomb
    friction_coefficient = 0.4
    friction_projection_degree = ONE
  []
[]

[BCs]
  # Small tangential sliding to activate c_tangential
  [topx]
    function := '0.05 * t'
  []
[]
