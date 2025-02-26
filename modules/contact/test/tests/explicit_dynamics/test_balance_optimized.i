[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Mesh]
  [block_one]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
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
  patch_update_strategy = auto
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
  [gap_rate]
  []
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
    execute_on = 'TIMESTEP_END'
  []
  [vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
    execute_on = 'TIMESTEP_END'
  []
  [accel_y]
    type = TestNewmarkTI
    variable = accel_y
    displacement = disp_y
    first = false
    execute_on = 'TIMESTEP_END'
  []
  [vel_y]
    type = TestNewmarkTI
    variable = vel_y
    displacement = disp_x
    execute_on = 'TIMESTEP_END'
  []
  [accel_z]
    type = TestNewmarkTI
    variable = accel_z
    displacement = disp_z
    first = false
    execute_on = 'TIMESTEP_END'
  []
  [vel_z]
    type = TestNewmarkTI
    variable = vel_z
    displacement = disp_z
    execute_on = 'TIMESTEP_END'
  []
[]

[Kernels]
  [DynamicTensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    volumetric_locking_correction = true
    stiffness_damping_coefficient = 0.001
    decomposition_method = EigenSolution
  []

  [Mass_x]
    type = MassMatrix
    variable = disp_x
    density = density
    matrix_tags = 'mass'
  []
  [Mass_y]
    type = MassMatrix
    variable = disp_y
    density = density
    matrix_tags = 'mass'
  []
  [Mass_z]
    type = MassMatrix
    variable = disp_z
    density = density
    matrix_tags = 'mass'
  []
[]

[Kernels]
  [gravity]
    type = Gravity
    variable = disp_z
    value = -981.0
  []
[]

[BCs]
  [x_front]
    type = DirectDirichletBC
    variable = disp_x
    boundary = 'ball_front'
    value = 0.0
  []
  [y_front]
    type = DirectDirichletBC
    variable = disp_y
    boundary = 'ball_front'
    value = 0.0
  []
  [x_fixed]
    type = DirectDirichletBC
    variable = disp_x
    boundary = 'base_back'
    value = 0.0
  []
  [y_fixed]
    type = DirectDirichletBC
    variable = disp_y
    boundary = 'base_back'
    value = 0.0
  []
  [z_fixed]
    type = DirectDirichletBC
    variable = disp_z
    boundary = 'base_back'
    value = 0.0
  []
  [z_fixed_front]
    type = DirectDirichletBC
    variable = disp_z
    boundary = 'base_front'
    value = 0.0
  []
[]

[ExplicitDynamicsContact]
  [my_contact]
    model = frictionless_balance
    primary = base_front
    secondary = ball_back
    vel_x = 'vel_x'
    vel_y = 'vel_y'
    vel_z = 'vel_z'
  []
[]

[Materials]
  [elasticity_tensor_block_one]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.0
    block = 1
    constant_on = SUBDOMAIN
  []
  [elasticity_tensor_block_two]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.0
    block = 2
    constant_on = SUBDOMAIN
  []
  [strain_block]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
    implicit = false
  []
  [stress_block]
    type = ComputeFiniteStrainElasticStress
  []
  [density_one]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1e1
    output_properties = 'density'
    block = '1'
  []
  [density_two]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1e6
    output_properties = 'density'
    block = '2'
  []
  [wave_speed]
    type = WaveSpeed
    outputs = 'exodus'
    output_properties = 'wave_speed'
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 0.0025
  dt = 0.00001
  timestep_tolerance = 1e-6
  [TimeIntegrator]
    type = DirectCentralDifference
    mass_matrix_tag = 'mass'
    use_constant_mass = true
  []
[]
[Outputs]
  interval = 10
  exodus = true
  csv = true
  file_base = test_balance_out
[]
