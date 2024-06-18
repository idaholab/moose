# One element test to test the central difference time integrator in 3D.
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [block_one]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmin = 4.5
    xmax = 5.5
    ymin = 4.5
    ymax = 5.5
    zmin = 0.0001
    zmax = 1.0001
    boundary_name_prefix = 'ball'
  []
  [block_two]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmin = 0.0
    xmax = 10
    ymin = 0.0
    ymax = 10
    zmin = -2
    zmax = 0
    boundary_name_prefix = 'base'
    boundary_id_offset = 10
  []
  [block_one_id]
    type = SubdomainIDGenerator
    input = block_one
    subdomain_id = 1
  []
  [block_two_id]
    type = SubdomainIDGenerator
    input = block_two
    subdomain_id = 2
  []
  [combine]
    type = MeshCollectionGenerator
    inputs = ' block_one_id block_two_id'
  []
  allow_renumbering = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxVariables]
  [vel_x]
  []
  [accel_x]
  []
  [vel_y]
  []
  [accel_y]
  []
  [vel_z]
  []
  [accel_z]
  []
[]

[AuxKernels]
  [accel_x]
    type = TestNewmarkTI
    variable = accel_x
    displacement = disp_x
    first = false
  []
  [vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
  []
  [accel_y]
    type = TestNewmarkTI
    variable = accel_y
    displacement = disp_y
    first = false
  []
  [vel_y]
    type = TestNewmarkTI
    variable = vel_y
    displacement = disp_x
  []
  [accel_z]
    type = TestNewmarkTI
    variable = accel_z
    displacement = disp_z
    first = false
  []
  [vel_z]
    type = TestNewmarkTI
    variable = vel_z
    displacement = disp_z
  []
[]

[Kernels]
  [DynamicTensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    volumetric_locking_correction = true
    stiffness_damping_coefficient = 0.04
    #generate_output = 'stress_zz strain_zz'
  []
  [inertia_x]
    type = InertialForce
    variable = disp_x
  []
  [inertia_y]
    type = InertialForce
    variable = disp_y
  []
  [inertia_z]
    type = InertialForce
    variable = disp_z
  []
[]

[Functions]
  [dispz]
    type = ParsedFunction
    expression = if(t<1.0e3,-0.01*t,0)
  []
  [push]
    type = ParsedFunction
    expression = if(t<10.0,0.01*t,0.1)
  []
[]

[BCs]
  [z_front]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'ball_front'
    function = dispz
    preset = false
  []
  [x_front]
    type = DirichletBC
    variable = disp_x
    boundary = 'ball_front'
    preset = false
    value = 0.0
  []
  [y_front]
    type = DirichletBC
    variable = disp_y
    boundary = 'ball_front'
    preset = false
    value = 0.0
  []
  [x_fixed]
    type = DirichletBC
    variable = disp_x
    boundary = 'base_back'
    preset = false
    value = 0.0
  []
  [y_fixed]
    type = DirichletBC
    variable = disp_y
    boundary = 'base_back'
    preset = false
    value = 0.0
  []
  [z_fixed]
    type = DirichletBC
    variable = disp_z
    boundary = 'base_back'
    preset = false
    value = 0.0
  []
[]

[ExplicitDynamicsContact]
  [my_contact]
    model = frictionless
    primary = base_front
    secondary = ball_back
    penalty = 1.0e3
  []
[]

[Materials]
  [elasticity_tensor_block_one]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e3
    poissons_ratio = 0.0
    block = 1
  []
  [elasticity_tensor_block_two]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.0
    block = 2
  []
  [strain_block]
    type = ComputeIncrementalStrain
    displacements = 'disp_x disp_y disp_z'
    implicit = false
  []
  [stress_block]
    type = ComputeFiniteStrainElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1e4
  []
  [wave_speed]
    type = WaveSpeed
  []
[]

[Executioner]
  type = Transient
  start_time = -0.01
  end_time = 0.25
  dt = 0.005
  timestep_tolerance = 1e-6

  [TimeIntegrator]
    type = CentralDifference
  []
[]

[Postprocessors]
  [disp_58z]
    type = NodalVariableValue
    nodeid = 1
    variable = disp_z
  []
  [critical_time_step]
    type = CriticalTimeStep
  []
  [contact_pressure_max]
    type = NodalExtremeValue
    variable = contact_pressure
    block = '1 2'
    value_type = max
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
