# Test for  HHT time integration

# The test is for an 1D bar element of  unit length fixed on one end
# with a ramped pressure boundary condition applied to the other end.
# alpha, beta and gamma are HHT time integration parameters
# The equation of motion in terms of matrices is:
#
# M*accel + alpha*(K*disp - K*disp_old) + K*disp = P(t+alpha dt)*Area
#
# Here M is the mass matrix, K is the stiffness matrix, P is the applied pressure
#
# This equation is equivalent to:
#
# density*accel + alpha*(Div stress - Div stress_old) +Div Stress= P(t+alpha dt)
#
# The first term on the left is evaluated using the Inertial force kernel
# The next two terms on the left involving alpha are evaluated using the
# DynamicStressDivergenceTensors Kernel
# The residual due to Pressure is evaluated using Pressure boundary condition
#
# The system will come to steady state slowly after the pressure becomes constant.
# Alpha equal to zero will result in Newmark integration.

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0.0
  xmax = 0.1
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 0.1
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
  [./accel_x]
  [../]
  [./vel_y]
  [../]
  [./accel_y]
  [../]
  [./vel_z]
  [../]
  [./accel_z]
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]

[Kernels]
  [./DynamicTensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    hht_alpha = 0.11
  [../]
  [./inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    beta = 0.25
    gamma = 0.5
  [../]

[]

[AuxKernels]
  [./accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./accel_z]
    type = NewmarkAccelAux
    variable = accel_z
    displacement = disp_z
    velocity = vel_z
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_z]
    type = NewmarkVelAux
    variable = vel_z
    acceleration = accel_z
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 0
    index_j = 1
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 0
    index_j = 1
  [../]

[]


[BCs]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value=0.0
  [../]
  [./top_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value=0.0
  [../]
  [./top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value=0.0
  [../]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value=0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value=0.0
  [../]
  [./Pressure]
    [./Side1]
      boundary = bottom
      function = pressure
      factor = 1
      hht_alpha = 0.11
      displacements = 'disp_x disp_y disp_z'
    [../]
  [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    fill_method = symmetric_isotropic
    C_ijkl = '210e9 0'
  [../]

  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]

  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'density'
    prop_values = '7750'
  [../]

[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 2
  dt = 0.1
[]


[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2 1.0 2.0 5.0'
    y = '0.0 0.1 0.2 1.0 1.0 1.0'
    scale_factor = 1e9
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
  [./disp]
    type = NodalExtremeValue
    variable = disp_y
    boundary = bottom
  [../]
  [./vel]
    type = NodalExtremeValue
    variable = vel_y
    boundary = bottom
  [../]
  [./accel]
    type = NodalExtremeValue
    variable = accel_y
    boundary = bottom
  [../]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  [../]
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
