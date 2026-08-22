!include frictionless_physical.i

[GlobalParams]
  # scaling=1/E so physical c constants (scaled by scalingFactor ~ 1/E) are
  # far smaller than the defaults (c_normal=1e6, c_tangential=1), making the
  # convergence gap between physical and default clearly observable.
  scaling := 1e-4
  degree_one_friction_residual = true
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
    # Physical (derived) c_tangential is this fixture's default, on top of the inherited physical
    # c_normal default: together they are the configuration that actually converges. The
    # fixed-constant (c_tangential_strategy = user) comparison behavior is exercised by overriding
    # this back via the command line, not by making it the default here, so that a plain run of
    # this file is always a working input.
    c_tangential_strategy = physical
  []
[]

[BCs]
  # Small tangential sliding to activate c_tangential
  [topx]
    function := '0.05 * t'
  []
[]
