# Test for damped small strain euler beam vibration in y direction

# An impulse load is applied at the end of a cantilever beam of length 4m.
# The properties of the cantilever beam are as follows:
# Young's modulus (E) = 1e4
# Shear modulus (G) = 4e7
# Shear coefficient (k) = 1.0
# Cross-section area (A) = 0.01
# Iy = 1e-4 = Iz
# Length (L)= 4 m
# density (rho) = 1.0
# mass proportional rayleigh damping(eta) = 0.1
# stiffness proportional rayleigh damping(eta) = 0.1
# HHT time integration parameter (alpha) = -0.3
# Corresponding Newmark beta time integration parameters beta = 0.4225 and gamma = 0.8

# For this beam, the dimensionless parameter alpha = kAGL^2/EI = 6.4e6
# Therefore, the behaves like a Euler-Bernoulli beam.

# The displacement time history from this analysis matches with that obtained from Abaqus.

# Values from the first few time steps are as follows:
# time  disp_y                vel_y                accel_y
# 0.0   0.0                   0.0                  0.0
# 0.2   0.019898364318588     0.18838688112273     1.1774180070171
# 0.4   0.045577003505278     0.087329917525455   -0.92596052423724
# 0.6   0.063767907208218     0.084330765885995    0.21274543331268
# 0.8   0.073602908614573     0.020029576220975   -0.45506879373455
# 1.0   0.06841704414745     -0.071840076837194   -0.46041813317992

[Mesh]
  type = GeneratedMesh
  nx = 10
  dim = 1
  xmin = 0.0
  xmax = 4.0
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./vel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./vel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./vel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./accel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_vel_z]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_x]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_y]
  order = FIRST
  family = LAGRANGE
  [../]
  [./rot_accel_z]
  order = FIRST
  family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./accel_x] # These auxkernels are only to check output
    type = TestNewmarkTI
    displacement = disp_x
    variable = accel_x
    first = false
  [../]
  [./accel_y]
    type = TestNewmarkTI
    displacement = disp_y
    variable = accel_y
    first = false
  [../]
  [./accel_z]
    type = TestNewmarkTI
    displacement = disp_z
    variable = accel_z
    first = false
  [../]
  [./vel_x]
    type = TestNewmarkTI
    displacement = disp_x
    variable = vel_x
  [../]
  [./vel_y]
    type = TestNewmarkTI
    displacement = disp_y
    variable = vel_y
  [../]
  [./vel_z]
    type = TestNewmarkTI
    displacement = disp_z
    variable = vel_z
  [../]
  [./rot_accel_x]
    type = TestNewmarkTI
    displacement = rot_x
    variable = rot_accel_x
    first = false
  [../]
  [./rot_accel_y]
    type = TestNewmarkTI
    displacement = rot_y
    variable = rot_accel_y
    first = false
  [../]
  [./rot_accel_z]
    type = TestNewmarkTI
    displacement = rot_z
    variable = rot_accel_z
    first = false
  [../]
  [./rot_vel_x]
    type = TestNewmarkTI
    displacement = rot_x
    variable = rot_vel_x
  [../]
  [./rot_vel_y]
    type = TestNewmarkTI
    displacement = rot_y
    variable = rot_vel_y
  [../]
  [./rot_vel_z]
    type = TestNewmarkTI
    displacement = rot_z
    variable = rot_vel_z
  [../]
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
[]

[Functions]
  [./force]
    type = PiecewiseLinear
    x = '0.0 0.2 0.4 10.0'
    y = '0.0 0.01  0.0  0.0'
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
  line_search = 'none'
  l_tol = 1e-11
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  start_time = 0.0
  dt = 0.2
  end_time = 5.0
  timestep_tolerance = 1e-6

  # Time integrator
  [./TimeIntegrator]
    type = NewmarkBeta
    beta = 0.4225
    gamma = 0.8
  [../]
[]

[Kernels]
  [./solid_disp_x]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
    zeta = 0.1
    alpha = -0.3
  [../]
  [./solid_disp_y]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
    zeta = 0.1
    alpha = -0.3
  [../]
  [./solid_disp_z]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
    zeta = 0.1
    alpha = -0.3
  [../]
  [./solid_rot_x]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
    zeta = 0.1
    alpha = -0.3
  [../]
  [./solid_rot_y]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
    zeta = 0.1
    alpha = -0.3
  [../]
  [./solid_rot_z]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
    zeta = 0.1
    alpha = -0.3
  [../]
  [./inertial_force_x]
    type = InertialForceBeam
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    eta = 0.1
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 0
    variable = disp_x
    alpha = -0.3
  [../]
  [./inertial_force_y]
    type = InertialForceBeam
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    eta = 0.1
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 1
    variable = disp_y
    alpha = -0.3
  [../]
  [./inertial_force_z]
    type = InertialForceBeam
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    eta = 0.1
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 2
    variable = disp_z
    alpha = -0.3
  [../]
  [./inertial_force_rot_x]
    type = InertialForceBeam
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    eta = 0.1
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 3
    variable = rot_x
    alpha = -0.3
  [../]
  [./inertial_force_rot_y]
    type = InertialForceBeam
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    eta = 0.1
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 4
    variable = rot_y
    alpha = -0.3
  [../]
  [./inertial_force_rot_z]
    type = InertialForceBeam
    block = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    eta = 0.1
    area = 0.01
    Iy = 1e-4
    Iz = 1e-4
    Ay = 0.0
    Az = 0.0
    component = 5
    variable = rot_z
    alpha = -0.3
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
  [./strain]
    type = ComputeIncrementalBeamStrain
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 0.01
    Ay = 0.0
    Az = 0.0
    Iy = 1.0e-4
    Iz = 1.0e-4
    y_orientation = '0.0 1.0 0.0'
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 0
  [../]
  [./density]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'density'
    prop_values = '1.0'
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
  file_base = 'dyn_euler_small_rayleigh_hht_out'
  exodus = true
  csv = true
  perf_graph = true
[]
