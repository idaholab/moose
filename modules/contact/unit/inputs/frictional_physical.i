!include frictionless_physical.i

[GlobalParams]
  # scaling=1/E so physical c constants (scaled by scalingFactor ~ 1/E) are
  # far smaller than the defaults (c_normal=1e6, c_tangential=1), making the
  # convergence gap between physical and default clearly observable.
  scaling := 1e-4
[]

[Materials]
  [tensor]
    youngs_modulus := 1.0e4
  []
  [tensor_1000]
    youngs_modulus := 1e5
  []
[]

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
