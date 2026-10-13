[Mesh]
  type = MeshGeneratorMesh

  [cartesian]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.45 0.1 0.45'
    ix = '5 1 5'
    dy = '0.45 0.1 0.45'
    iy = '5 1 5'
    subdomain_id = '1 1 1
                    1 2 1
                    1 1 1'
  []

  [add_iss_1]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    new_boundary = 'interface'
    input = cartesian
  []

  [block_deleter]
    type = BlockDeletionGenerator
    block = 2
    input = add_iss_1
  []
[]

[Variables]
  [temperature]
    initial_condition = 300
  []
[]

[AuxVariables]
  [channel_T]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 400
  []

  [channel_Hw]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1000
  []
[]

[Kernels]
  [graphite_diffusion]
    type = KokkosHeatConduction
    variable = temperature
    thermal_conductivity = 'k_s'
  []
[]

[BCs]
  ## boundary conditions for the thm channels in the reflector
  [channel_heat_transfer]
    type = KokkosCoupledConvectiveHeatFluxBC
    variable = temperature
    htc = channel_Hw
    T_infinity = channel_T
    boundary = 'interface'
  []

  # hot boundary on the left
  [left]
    type = KokkosDirichletBC
    variable = temperature
    value = 1000
    boundary = 'left'
  []

  # cool boundary on the right
  [right]
    type = KokkosDirichletBC
    variable = temperature
    value = 300
    boundary = 'right'
  []
[]

[Materials]
  [thermal]
    type = KokkosGenericConstantMaterial
    prop_names = 'k_s'
    prop_values = '12'
  []

  [htc_material]
    type = KokkosGenericConstantMaterial
    prop_names = 'alpha_wall'
    prop_values = '1000'
  []

  [tfluid_mat]
    # equals the linear interpolation x = y = '400 500' of the reference input
    type = KokkosParsedMaterial
    property_name = tfluid_mat
    coupled_variables = channel_T
    expression = channel_T
  []
[]

[Postprocessors]
  [Qw1]
    type = KokkosConvectiveHeatTransferSideIntegral
    T_fluid_var = channel_T
    htc_var = channel_Hw
    T_solid = temperature
    boundary = interface
  []

  [Qw2]
    type = KokkosConvectiveHeatTransferSideIntegral
    T_fluid_var = channel_T
    htc = alpha_wall
    T_solid = temperature
    boundary = interface
  []

  [Qw3]
    type = KokkosConvectiveHeatTransferSideIntegral
    T_fluid = tfluid_mat
    htc = alpha_wall
    T_solid = temperature
    boundary = interface
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
