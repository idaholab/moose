[GlobalParams]
  gravity = '0 0 0'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmax = 1.1
  ymax = 1.1
  zmax = 1.1
  nx = 1
  ny = 1
  nz = 1
  elem_type = HEX27
[]

[Variables]
  [./vel_x]
    family = LAGRANGE
    order = SECOND
  [../]

  [./vel_y]
    family = LAGRANGE
    order = SECOND
  [../]

  [./vel_z]
    family = LAGRANGE
    order = SECOND
  [../]

  [./p]
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Kernels]
  [./x_momentum_time_supg]
    type = INSMomentumTimeDerivativeSUPG
    variable = vel_x
    u = vel_x
    v = vel_y
    w = vel_z
    component = 0
  [../]

  [./x_supg]
    type = INSMomentumSUPG
    variable = vel_x
    u = vel_x
    v = vel_y
    w = vel_z
    p = p
    component = 0
  [../]

  [./y_momentum_time_supg]
    type = INSMomentumTimeDerivativeSUPG
    variable = vel_y
    u = vel_x
    v = vel_y
    w = vel_z
    component = 1
  [../]

  [./y_supg]
    type = INSMomentumSUPG
    variable = vel_y
    u = vel_x
    v = vel_y
    p = p
    w = vel_z
    component = 1
  [../]

  [./z_momentum_time_supg]
    type = INSMomentumTimeDerivativeSUPG
    variable = vel_z
    u = vel_x
    v = vel_y
    w = vel_z
    component = 2
  [../]

  [./z_supg]
    type = INSMomentumSUPG
    variable = vel_z
    u = vel_x
    v = vel_y
    w = vel_z
    p = p
    component = 2
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '.5 1.1'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.1
  num_steps = 1
  petsc_options = '-snes_test_display'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'test'
[]

[ICs]
  [./u]
    type = RandomIC
    variable = vel_x
    min = 0.1
    max = 0.9
  [../]
  [./v]
    type = RandomIC
    variable = vel_y
    min = 0.1
    max = 0.9
  [../]
  [./w]
    type = RandomIC
    variable = vel_z
    min = 0.1
    max = 0.9
  [../]
  [./p]
    type = RandomIC
    variable = p
    min = 0.1
    max = 0.9
  [../]
[]
