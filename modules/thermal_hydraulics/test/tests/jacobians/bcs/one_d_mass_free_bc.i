[JacobianTestGeneral]
  variable_names = 'rhoA rhouA'
  variable_values = '100 100'
  snes_test_err = 1e-6
  use_transient_executioner = true
[]

[BCs]
  [bc_1]
    type = OneDMassFreeBC
    variable = rhoA
    boundary = 0
    normal = -1
    arhouA = rhouA
  []
  [bc_2]
    type = OneDMassFreeBC
    variable = rhoA
    boundary = 1
    normal = 1
    arhouA = rhouA
  []
[]
