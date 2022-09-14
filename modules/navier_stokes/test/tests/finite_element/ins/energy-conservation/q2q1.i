[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 10
    ny = 10
    dim = 2
  []
  [subdomain]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    block_id = 1
    input = gen
  []
  [break_boundary]
    input = subdomain
    type = BreakBoundaryOnSubdomainGenerator
    boundaries = 'bottom top'
  []
  [sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = break_boundary
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'fluid_left'
  []
  coord_type = RZ
  second_order = true
[]

[Variables]
  [T]
    order = SECOND
  []
  [velocity]
    family = LAGRANGE_VEC
    order = SECOND
    block = 1
  []
  [pressure]
    block = 1
  []
[]

[ICs]
  [vel]
    type = VectorConstantIC
    variable = velocity
    x_value = 1e-15
    y_value = 1e-15
    block = 1
  []
[]

[Kernels]
  [mass]
    type = INSADMass
    variable = pressure
    block = 1
  []

  [momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
    block = 1
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
    block = 1
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = pressure
    integrate_p_by_parts = true
    block = 1
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
    block = 1
  []

  [temperature_advection]
    type = INSADEnergyAdvection
    variable = T
     block = 1
  []
  [temperature_supg]
    type = INSADEnergySUPG
    variable = T
    velocity = velocity
    block = 1
  []
  [temperature_conduction]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = 'k'
  []
  [heat_source]
    type = BodyForce
    variable = T
    block = 0
    function = 'x + y'
  []
[]

[BCs]
  [velocity_inlet]
    type = VectorFunctionDirichletBC
    variable = velocity
    function_y = 1
    boundary = 'bottom_to_1'
  []
  [wall]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'fluid_left right'
  []
  [convective_heat_transfer]
    type = ConvectiveHeatFluxBC
    variable = T
    T_infinity = 0
    heat_transfer_coefficient = 1
    boundary = 'right'
  []
[]

[Materials]
  [constant]
    type = ADGenericConstantMaterial
    prop_names = 'cp rho k mu'
    prop_values = '1 1   1 1'
  []
  [ins]
    type = INSADStabilized3Eqn
    pressure = pressure
    velocity = velocity
    temperature = T
    block = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [convective_heat_transfer]
    type = ConvectiveHeatTransferSideIntegral
    T_solid = T
    T_fluid = 0
    htc = 1
    boundary = 'right'
  []
  [advection]
    type = INSADElementIntegralEnergyAdvection
    temperature = T
    velocity = velocity
    cp = cp
    rho = rho
    block = 1
  []
  [source]
    type = FunctionElementIntegral
    function = 'x + y'
    block = 0
  []
  [energy_balance]
    type = ParsedPostprocessor
    function = 'convective_heat_transfer + advection - source'
    pp_names = 'convective_heat_transfer advection source'
  []
[]
