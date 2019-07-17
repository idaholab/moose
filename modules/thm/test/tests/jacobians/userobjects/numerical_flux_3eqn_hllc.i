[JacobianTest1PhaseRDG]
  add_dg_kernel = true
  numerical_flux = flux
  ic_option = riemann_L
  snes_test_err = 1e-8
[]

[UserObjects]
  [./flux]
    type = NumericalFlux3EqnHLLC
    fluid_properties = fluid_properties
    execute_on = 'linear nonlinear'
  [../]
[]
