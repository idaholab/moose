# A single element is stretched.
#
# For all time:
# disp_x = 0
# disp_z = 3
#
# The velocities are initialised to zero
# The accelerations are initialised to
# accel_x = 0
# accel_y = 2
# accel_z = 0
#
# The only degree of freedom is disp_y.
# It is initialised to zero.
# The DE is the ZEROTH component of
# density * disp x accel = BodyForce
# (Choosing the zeroth component is unusual: this
# is to illustrate correct behaviour of the
# InertialTorque Kernel, rather than being
# relevant to any particular solid-mechanics problem.)
# The LHS = - density * disp_z * accel_y
# With density = 0.5 and BodyForce = -3 the solution is
# accel_y = 2, vel_y = 2 * t, and disp_y = t^2
[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  velocities = 'vel_x vel_y vel_z'
  accelerations = 'accel_x accel_y accel_z'
  gamma = 0.5
  beta = 0.25
  alpha = 0.0
  eta = 0.0
[]

[Variables]
  [./disp_y]
  [../]
[]

[Kernels]
  [./icm_x]
    type = InertialTorque
    component = 0
    variable = disp_y
  [../]
  [./source_x]
    type = BodyForce
    variable = disp_y
    function = -3
  [../]
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_z]
    initial_condition = 3
  [../]
  [./vel_x]
  [../]
  [./vel_y]
  [../]
  [./vel_z]
  [../]
  [./accel_x]
  [../]
  [./accel_y]
    initial_condition = 2
  [../]
  [./accel_z]
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

[BCs]
  # zmin is called back
  # zmax is called front
  # ymin is called bottom
  # ymax is called top
  # xmin is called left
  # xmax is called right
[]

[Materials]
  [./density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 0.5
  [../]
[]

[Postprocessors]
  [./y_disp]
    type = PointValue
    point = '0 0 0'
    use_displaced_mesh = false
    variable = disp_y
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres bjacobi 1E-15 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  num_steps = 10
[]

[Outputs]
  csv = true
[]
