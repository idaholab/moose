[GlobalParams]
  gravity = '0 0 0'
  laplace = true
  transient_term = true
  supg = true
  pspg = true
  family = LAGRANGE
  order = SECOND
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 0
  xmax = 1.1
  ymin = -1.1
  ymax = 1.1
  elem_type = QUAD9
[]

[Problem]
  coord_type = RZ
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = NEWTON
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1.1
  # petsc_options = '-snes_test_display'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'test'
[]

[Variables]
  [./vel_x]
    # Velocity in radial (r) direction
  [../]
  [./vel_y]
    # Velocity in axial (z) direction
  [../]
  [./p]
    order = FIRST
  [../]
[]


[Kernels]
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
  [../]
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
  [../]
  [./mass]
    type = INSMassRZ
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceFormRZ
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceFormRZ
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1.1 1.1'
  [../]
[]

[ICs]
  [./vel_x]
    type = RandomIC
    variable = vel_x
    min = 0.1
    max = 0.9
  [../]
  [./vel_y]
    type = RandomIC
    variable = vel_y
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

[Outputs]
  dofmap = true
[]
