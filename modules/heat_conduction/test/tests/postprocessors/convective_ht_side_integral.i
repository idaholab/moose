[Mesh]
  type = MeshGeneratorMesh

  [./cartesian]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.45 0.1 0.45'
    ix = '5 1 5'
    dy = '0.45 0.1 0.45'
    iy = '5 1 5'
    subdomain_id = '1 1 1
                    1 2 1
                    1 1 1'
  [../]

  [./add_iss_1]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    new_boundary = 'interface'
    input = cartesian
  [../]

  [./block_deleter]
    type = BlockDeletionGenerator
    block = 2
    input = add_iss_1
  [../]
[]

[Variables]
  [./temperature]
    initial_condition = 300
  [../]
[]

[AuxVariables]
  [./channel_T]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 400
  [../]

  [./channel_Hw]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1000
  [../]
[]

[Kernels]
  [./graphite_diffusion]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = 'k_s'
  [../]
[]

[BCs]
  ## boundary conditions for the thm channels in the reflector
  [./channel_heat_transfer]
    type = CoupledConvectiveHeatFluxBC
    variable = temperature
    htc = channel_Hw
    T_infinity = channel_T
    boundary = 'interface'
  [../]

  # hot boundary on the left
  [./left]
    type = DirichletBC
    variable = temperature
    value = 1000
    boundary = 'left'
  [../]

  # cool boundary on the right
  [./right]
    type = DirichletBC
    variable = temperature
    value = 300
    boundary = 'right'
  [../]
[]

[Materials]
  [./thermal]
    type = GenericConstantMaterial
    prop_names = 'k_s'
    prop_values = '12'
  [../]

  [./htc_material]
    type = GenericConstantMaterial
    prop_names = 'alpha_wall'
    prop_values = '1000'
  [../]

  [./tfluid_mat]
    type = PiecewiseLinearInterpolationMaterial
    property = tfluid_mat
    variable = channel_T
    x = '400 500'
    y = '400 500'
  [../]
[]

[Postprocessors]
  [./Qw1]
    type = ConvectiveHeatTransferSideIntegral
    T_fluid_var = channel_T
    htc_var = channel_Hw
    T_solid = temperature
    boundary = interface
  [../]

  [./Qw2]
    type = ConvectiveHeatTransferSideIntegral
    T_fluid_var = channel_T
    htc = alpha_wall
    T_solid = temperature
    boundary = interface
  [../]

  [./Qw3]
    type = ConvectiveHeatTransferSideIntegral
    T_fluid = tfluid_mat
    htc = alpha_wall
    T_solid = temperature
    boundary = interface
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
