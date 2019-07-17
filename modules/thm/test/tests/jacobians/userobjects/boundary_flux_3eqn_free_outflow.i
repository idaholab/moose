[JacobianTest1PhaseRDG]
  add_bc = true
  boundary_flux = flux
  ic_option = constant
  snes_test_err = 1e-8
[]

[UserObjects]
  [./flux]
    type = BoundaryFlux3EqnFreeOutflow
    fluid_properties = fluid_properties
    execute_on = 'linear nonlinear'
  [../]
[]
