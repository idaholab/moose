# This test demonstrates explicit contact with MOOSE and includes optimizations
# to enhance performance.
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Mesh]
  [block_one]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
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
    nx = 9
    ny = 9
    nz = 4
    xmin = 3
    xmax = 7
    ymin = 3
    ymax = 7
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
  # patch_update_strategy = always
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
    block = 1
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

[]

[ExplicitDynamicsContact]
  [my_contact]
    model = frictionless_balance
    primary = 'base_front ball_back'
    secondary = 'ball_back base_front'
    vel_x = 'vel_x'
    vel_y = 'vel_y'
    vel_z = 'vel_z'
  []
[]

[Materials]
  [elasticity_tensor_block_one]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e1
    poissons_ratio = 0.3
    block = 1
    constant_on = SUBDOMAIN
  []
  [elasticity_tensor_block_two]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e7
    poissons_ratio = 0.3
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
    prop_values = 1e7
    output_properties = 'density'
    block = '1'
  []
  [density_two]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1e3
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
  end_time = 0.04
  dt = 0.0001
  timestep_tolerance = 1e-6
  [TimeIntegrator]
    type = DirectCentralDifference
    mass_matrix_tag = 'mass'
    use_constant_mass = true
  []
  skip_exception_check = true
[]
[Outputs]
  interval = 100
  exodus = true
[]
