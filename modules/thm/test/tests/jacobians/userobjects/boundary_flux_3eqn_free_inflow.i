[JacobianTest1PhaseRDG]
  add_bc = true
  boundary_flux = flux
  ic_option = constant
  snes_test_err = 1e-6
[]

[UserObjects]
  [./flux]
    type = BoundaryFlux3EqnFreeInflow
    rho_infinity = 1
    vel_infinity = 2
    p_infinity = 1.5
    fluid_properties = fluid_properties
    execute_on = 'linear nonlinear'
  [../]
[]
