# This input file tests the jacobians of many of the INS kernels
[GlobalParams]
  gravity = '1.1 1.1 1.1'
  u = vel_x
  v = vel_y
  w = vel_z
  pressure = p
  integrate_p_by_parts = true
  laplace = true
  pspg = true
  supg = true
  alpha = 1.1
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 3.0
  ymin = 0
  ymax = 1.5
  zmax = 1.1
  nx = 1
  ny = 1
  nz = 1
  elem_type = HEX27
[]

[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]
  [./vel_y]
    order = SECOND
    family = LAGRANGE
  [../]
  [./vel_z]
    order = SECOND
    family = LAGRANGE
  [../]
  [./p]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./mass]
    type = INSMass
    variable = p
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    component = 0
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    component = 1
  [../]
  [./z_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_z
    component = 2
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '0.5 1.5'
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'test'
[]

[ICs]
  [./p]
    type = RandomIC
    variable = p
    min = 0.5
    max = 1.5
  [../]
  [./vel_x]
    type = RandomIC
    variable = vel_x
    min = 0.5
    max = 1.5
  [../]
  [./vel_y]
    type = RandomIC
    variable = vel_y
    min = 0.5
    max = 1.5
  [../]
  [./vel_z]
    type = RandomIC
    variable = vel_z
    min = 0.5
    max = 1.5
  [../]
[]
