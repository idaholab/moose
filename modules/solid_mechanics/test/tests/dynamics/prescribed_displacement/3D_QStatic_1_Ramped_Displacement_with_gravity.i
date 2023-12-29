# One 3D element under ramped displacement loading.
#
# loading in z direction:
# time : 0.0 0.1  0.2  0.3
# disp : 0.0 0.0 -0.01 -0.01

# Gravity is applied in y direction. To equilibrate the system
# under gravity, a static analysis is run in the first time step
# by turning off the inertial terms. (see controls block and
# DynamicTensorMechanics block).

# Result: The displacement at the top node in the z direction should match
# the prescribed displacement. Also, the z acceleration should
# be two triangular pulses, one peaking at 0.1 and another peaking at
# 0.2.

# The y displacement would be offset by the gravity displacement.
# Also the y acceleration and velocity should be zero until the loading in
# the z direction starts (i.e, until 0.1s)

# Note: The time step used in the displacement data file should match
# the simulation time step (dt and dtmin in the Executioner block).

[Mesh]
  type = GeneratedMesh
  dim = 3 # Dimension of the mesh
  nx = 1 # Number of elements in the x direction
  ny = 1 # Number of elements in the y direction
  nz = 1 # Number of elements in the z direction
  xmin = 0.0
  xmax = 1
  ymin = 0.0
  ymax = 1
  zmin = 0.0
  zmax = 1
  allow_renumbering = false # So NodalVariableValue can index by id
[]

[Variables] # variables that are solved
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables] # variables that are calculated for output
  [./accel_x]
  [../]
  [./vel_x]
  [../]
  [./accel_y]
  [../]
  [./vel_y]
  [../]
  [./accel_z]
  [../]
  [./vel_z]
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./DynamicTensorMechanics] # zeta*K*vel + K * disp
    displacements = 'disp_x disp_y disp_z'
    stiffness_damping_coefficient = 0.000025
    static_initialization = true #turns off rayliegh damping for the first time step to stabilize system under gravity
  [../]
  [./inertia_x] # M*accel + eta*M*vel
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25 # Newmark time integration
    gamma = 0.5 # Newmark time integration
    eta = 19.63
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
    eta = 19.63
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    beta = 0.25
    gamma = 0.5
    eta = 19.63
  [../]
  [./gravity]
    type = Gravity
    variable = disp_y
    value = -9.81
  [../]
[]

[AuxKernels]
  [./accel_x] # Calculates and stores acceleration at the end of time step
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_x] # Calculates and stores velocity at the end of the time step
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
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
  [../]
[]

[Functions]
  [./displacement_front]
    type = PiecewiseLinear
    data_file = 'displacement.csv'
    format = columns
  [../]
[]

[BCs]
  [./prescribed_displacement]
    type = PresetDisplacement
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    beta = 0.25
    boundary = front
    function = displacement_front
  [../]
  [./anchor_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./anchor_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./anchor_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    youngs_modulus = 325e6 #Pa
    poissons_ratio = 0.3
    type = ComputeIsotropicElasticityTensor
    block = 0
  [../]
  [./strain]
    #Computes the strain, assuming small strains
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    #Computes the stress, using linear elasticity
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = density
    prop_values = 2000 #kg/m3
  [../]
[]

[Controls] # turns off inertial terms for the first time step
  [./period0]
    type = TimePeriod
    disable_objects = '*/vel_x */vel_y */vel_z */accel_x */accel_y */accel_z */inertia_x */inertia_y */inertia_z'
    start_time = 0.0
    end_time = 0.1 # dt used in the simulation
  [../]
[../]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 3.0
  l_tol = 1e-6
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  dt = 0.1
  timestep_tolerance = 1e-6
[]

[Postprocessors] # These quantites are printed to a csv file at every time step
  [./_dt]
    type = TimestepSize
  [../]
  [./accel_6x]
    type = NodalVariableValue
    nodeid = 6
    variable = accel_x
  [../]
  [./accel_6y]
    type = NodalVariableValue
    nodeid = 6
    variable = accel_y
  [../]
  [./accel_6z]
    type = NodalVariableValue
    nodeid = 6
    variable = accel_z
  [../]
  [./vel_6x]
    type = NodalVariableValue
    nodeid = 6
    variable = vel_x
  [../]
  [./vel_6y]
    type = NodalVariableValue
    nodeid = 6
    variable = vel_y
  [../]
  [./vel_6z]
    type = NodalVariableValue
    nodeid = 6
    variable = vel_z
  [../]
  [./disp_6x]
    type = NodalVariableValue
    nodeid = 6
    variable = disp_x
  [../]
  [./disp_6y]
    type = NodalVariableValue
    nodeid = 6
    variable = disp_y
  [../]
  [./disp_6z]
    type = NodalVariableValue
    nodeid = 6
    variable = disp_z
  [../]
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
