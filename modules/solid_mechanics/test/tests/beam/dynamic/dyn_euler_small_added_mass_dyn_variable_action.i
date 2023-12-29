# Test for small strain euler beam vibration in y direction
# The velocity and acceleration AuxVariables and the corresponding AuxKernels
# are set up using the LineElementAction using add_dynamic_variables. The action
# also creates the displacement variables, stress divergence kernels and
# beam strain. NodalTranslationalInertia is not created by the action.

# An impulse load is applied at the end of a cantilever beam of length 4m.
# The beam is massless with a lumped mass at the end of the beam
# The properties of the cantilever beam are as follows:
# Young's modulus (E) = 1e4
# Shear modulus (G) = 4e7
# Shear coefficient (k) = 1.0
# Cross-section area (A) = 0.01
# Iy = 1e-4 = Iz
# Length (L)= 4 m
# mass (m) = 0.01899772

# For this beam, the dimensionless parameter alpha = kAGL^2/EI = 6.4e6
# Therefore, the beam behaves like a Euler-Bernoulli beam.

# The theoretical first frequency of this beam is:
# f1 = 1/(2 pi) * sqrt(3EI/(mL^3)) = 0.25

# This implies that the corresponding time period of this beam is 4s.

# The FEM solution for this beam with 10 element gives time periods of 4s with time step of 0.01s.
# A higher time step of 0.1 s is used in the test to reduce computational time.

# The time history from this analysis matches with that obtained from Abaqus.

# Values from the first few time steps are as follows:
# time   disp_y                vel_y                accel_y
# 0.0    0.0                   0.0                  0.0
# 0.1    0.0013076435060869    0.026152870121738    0.52305740243477
# 0.2    0.0051984378734383    0.051663017225289   -0.01285446036375
# 0.3    0.010269120909367     0.049750643493289   -0.02539301427625
# 0.4    0.015087433925158     0.046615616822532   -0.037307519138892
# 0.5    0.019534963888307     0.042334982440433   -0.048305168503101

[Mesh]
  type = GeneratedMesh
  xmin = 0.0
  xmax = 4.0
  nx = 10
  dim = 1
  displacements = 'disp_x disp_y disp_z'
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_y2]
    type = UserForcingFunctionNodalKernel
    variable = disp_y
    boundary = right
    function = force
  [../]
  [./x_inertial]
    type = NodalTranslationalInertia
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    boundary = right
    beta = 0.25
    gamma = 0.5
    mass = 0.01899772
  [../]
  [./y_inertial]
    type = NodalTranslationalInertia
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    boundary = right
    beta = 0.25
    gamma = 0.5
    mass = 0.01899772
  [../]
  [./z_inertial]
    type = NodalTranslationalInertia
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    boundary = right
    beta = 0.25
    gamma = 0.5
    mass = 0.01899772
  [../]
[]

[Functions]
  [./force]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2 10.0'
    y = '0.0 1e-2  0.0  0.0'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  petsc_options_iname = '-ksp_type -pc_type'
  petsc_options_value = 'preonly   lu'

  dt = 0.1
  end_time = 5.0
  timestep_tolerance = 1e-6
[]

[Modules/TensorMechanics/LineElementMaster]
  [./all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'

    # Geometry parameters
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    y_orientation = '0.0 1.0 0.0'

    # Add AuxVariables and AuxKernels for dynamic simulation
    add_dynamic_variables = true

    velocities = 'vel_x vel_y vel_z'
    accelerations = 'accel_x accel_y accel_z'
    rotational_velocities = 'rot_vel_x rot_vel_y rot_vel_z'
    rotational_accelerations = 'rot_accel_x rot_accel_y rot_accel_z'

    beta = 0.25 # Newmark time integration parameter
    gamma = 0.5 # Newmark time integration parameter
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 1.0e4
    poissons_ratio = -0.999875
    shear_coefficient = 1.0
    block = 0
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 0
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_y]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = disp_y
  [../]
  [./vel_y]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = vel_y
  [../]
  [./accel_y]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = accel_y
  [../]
[]

[Outputs]
  file_base = 'dyn_euler_small_added_mass_out'
  hide = 'rot_vel_x rot_vel_y rot_vel_z rot_accel_x rot_accel_y rot_accel_z'
  exodus = true
  csv = true
[]
