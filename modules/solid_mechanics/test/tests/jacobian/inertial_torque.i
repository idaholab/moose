# Check of the InertialTorque Jacobian
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  velocities = 'vel_x vel_y vel_z'
  accelerations = 'accel_x accel_y accel_z'
  gamma = 0.4
  beta = 0.4
  alpha = 0.1
  eta = 0.1
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./vel_x]
  [../]
  [./vel_y]
  [../]
  [./vel_z]
  [../]
  [./accel_x]
  [../]
  [./accel_y]
  [../]
  [./accel_z]
  [../]
[]

[ICs]
  [./disp_x]
    type = RandomIC
    variable = disp_x
  [../]
  [./disp_y]
    type = RandomIC
    variable = disp_y
  [../]
  [./disp_z]
    type = RandomIC
    variable = disp_z
  [../]
  [./vel_x]
    type = RandomIC
    variable = vel_x
  [../]
  [./vel_y]
    type = RandomIC
    variable = vel_y
  [../]
  [./vel_z]
    type = RandomIC
    variable = vel_z
  [../]
  [./accel_x]
    type = RandomIC
    variable = accel_x
  [../]
  [./accel_y]
    type = RandomIC
    variable = accel_y
  [../]
  [./accel_z]
    type = RandomIC
    variable = accel_z
  [../]
[]


[Kernels]
  [./icm_x]
    type = InertialTorque
    component = 0
    variable = disp_x
  [../]
  [./icm_y]
    type = InertialTorque
    component = 1
    variable = disp_y
  [../]
  [./icm_z]
    type = InertialTorque
    component = 2
    variable = disp_z
  [../]
[]


[AuxKernels]
  [./vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    execute_on = timestep_end
  [../]
  [./vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    execute_on = timestep_end
  [../]
  [./vel_z]
    type = NewmarkVelAux
    variable = vel_z
    acceleration = accel_z
    execute_on = timestep_end
  [../]
  [./accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    execute_on = timestep_end
  [../]
  [./accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    execute_on = timestep_end
  [../]
  [./accel_z]
    type = NewmarkAccelAux
    variable = accel_z
    displacement = disp_z
    velocity = vel_z
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 3.0
  [../]
[]


[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type'
    petsc_options_value = 'test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
[]
