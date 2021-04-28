[JacobianTestGeneral]
  variable_names = 'rhoA'
  variable_values = '100'
  aux_variable_names = 'A'
  aux_variable_values = '1e-1'
  snes_test_err = 1e-6
[]

[BCs]
  [bc_1]
    type = OneDMassHRhoUBC
    variable = rhoA
    boundary = 0
    normal = -1
    rhou = 123
    A = A
  []
  [bc_2]
    type = OneDMassHRhoUBC
    variable = rhoA
    boundary = 1
    normal = 1
    rhou = 123
    A = A
  []
[]
