# Test for small strain euler beam vibration in y direction

# An impulse load is applied at the end of a cantilever beam of length 5ft (60 in).
# The beam is massless with a lumped mass at the end of the beam of 5000 lb
# The properties of the cantilever beam are as follows:
# E = 1e7 and I = 120 in^4
# Assuming a square cross section A = sqrt(12 * I) = 37.95
# Shear modulus (G) = 3.846e6
# Shear coefficient (k) = 1.0
# Cross-section area (A) = 1.0
# mass (m) = 5000 lb / 386 = 12.95

# The theoretical first frequency of this beam is:
# f1 = 1/(2 pi) * sqrt(3EI/(mL^3)) = 5.71 cps

# This implies that the corresponding time period of this beam is 0.175 s.

# The FEM solution for this beam with 10 elements gives
# a time period of 0.175 s with time step of 0.005 s.

# Reference: Strength of Materials by Marin ans Sauer, 2nd Ed.
# Example Problem 11-50, pg. 375

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0.0
  xmax = 60.0
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
    mass = 12.95
  [../]
  [./y_inertial]
    type = NodalTranslationalInertia
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    boundary = right
    beta = 0.25
    gamma = 0.5
    mass = 12.95
  [../]
  [./z_inertial]
    type = NodalTranslationalInertia
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    boundary = right
    beta = 0.25
    gamma = 0.5
    mass = 12.95
  [../]
[]

[Functions]
  [./force]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2 10.0'
    y = '0.0 1e2  0.0  0.0'
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
  l_tol = 1e-8
  l_max_its = 50
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  start_time = 0.0
  dt = 0.005
  end_time = 1.5
  timestep_tolerance = 1e-6
[]

[Kernels]
  [./solid_disp_x]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
  [../]
  [./solid_disp_y]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
  [../]
  [./solid_disp_z]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
  [../]
  [./solid_rot_x]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
  [../]
  [./solid_rot_y]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
  [../]
  [./solid_rot_z]
    type = StressDivergenceBeam
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 1.0e7
    poissons_ratio = 0.30005200208
    shear_coefficient = 1.0
    block = 0
  [../]
  [./strain]
    type = ComputeIncrementalBeamStrain
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 37.95
    Ay = 0.0
    Az = 0.0
    Iy = 120.0
    Iz = 120.0
    y_orientation = '0.0 1.0 0.0'
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 0
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = PointValue
    point = '60.0 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_y]
    type = PointValue
    point = '60.0 0.0 0.0'
    variable = disp_y
  [../]
  [./vel_y]
    type = PointValue
    point = '60.0 0.0 0.0'
    variable = vel_y
  [../]
  [./accel_y]
    type = PointValue
    point = '60.0 0.0 0.0'
    variable = accel_y
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  perf_graph = true
[]
