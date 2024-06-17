# Test for the Newmark-Beta time integrator

[Mesh]
  [./generated_mesh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 2
    xmin = 0.0
    xmax = 1
    ymin = 0.0
    ymax = 1
    zmin = 0.0
    zmax = 2
  [../]
  [./all_nodes]
    type = BoundingBoxNodeSetGenerator
    new_boundary = 'all'
    input = 'generated_mesh'
    top_right = '1 1 2'
    bottom_left = '0 0 0'
  [../]
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
[]

[Kernels]
  [./DynamicSolidMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[AuxKernels]
  [./accel_x]
    type = TestNewmarkTI
    variable = accel_x
    displacement = disp_x
    first = false
  [../]
  [./vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
  [../]
  [./accel_y]
    type = TestNewmarkTI
    variable = accel_y
    displacement = disp_y
    first = false
  [../]
  [./vel_y]
    type = TestNewmarkTI
    variable = vel_y
    displacement = disp_y
  [../]
  [./accel_z]
    type = TestNewmarkTI
    variable = accel_z
    displacement = disp_z
    first = false
  [../]
  [./vel_z]
    type = TestNewmarkTI
    variable = vel_z
    displacement = disp_z
  [../]
[]

[BCs]
  [./x_bot]
    type = PresetDisplacement
    boundary = 'back'
    variable = disp_x
    beta = 0.25
    velocity = vel_x
    acceleration = accel_x
    function = dispx
  [../]
  [./y_bot]
    type = PresetDisplacement
    boundary = 'back'
    variable = disp_y
    beta = 0.25
    velocity = vel_y
    acceleration = accel_y
    function = dispy
  [../]
  [./z_bot]
    type = PresetDisplacement
    boundary = 'back'
    variable = disp_z
    beta = 0.25
    velocity = vel_z
    acceleration = accel_z
    function = dispz
  [../]
[]

[Functions]
  [./dispx]
    type = PiecewiseLinear
    x = '0.0 1.0 2.0 3.0 4.0' # time
    y = '0.0 1.0 0.0 -1.0 0.0'  # displacement
  [../]
  [./dispy]
    type = ParsedFunction
    expression = 0.1*t*t*sin(10*t)
  [../]
  [./dispz]
    type = ParsedFunction
    expression = 0.1*t*t*sin(20*t)
  [../]
[]

[NodalKernels]
  [./nodal_mass_x]
    type = NodalTranslationalInertia
    boundary = 'all'
    nodal_mass_file = 'nodal_mass_file.csv'
    variable = 'disp_x'
  [../]
  [./nodal_mass_y]
    type = NodalTranslationalInertia
    boundary = 'all'
    nodal_mass_file = 'nodal_mass_file.csv'
    variable = 'disp_y'
  [../]
  [./nodal_mass_z]
    type = NodalTranslationalInertia
    boundary = 'all'
    nodal_mass_file = 'nodal_mass_file.csv'
    variable = 'disp_z'
  [../]
[]

[Materials]
  [./elasticity_tensor_block]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.25
    block = 0
  [../]
  [./strain_block]
    type = ComputeIncrementalStrain
    block = 0
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress_block]
    type = ComputeFiniteStrainElasticStress
    block = 0
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_abs_tol = 1e-08
  nl_rel_tol = 1e-08
  timestep_tolerance = 1e-6
  start_time = -0.01
  end_time = 0.1
  dt = 0.005
  [./TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  [../]
[]

[Postprocessors]
  [./accel_10x]
    type = NodalVariableValue
    nodeid = 10
    variable = accel_x
  [../]
[]

[Outputs]
  exodus = false
  csv = true
[]
