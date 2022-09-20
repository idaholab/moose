# Test the properties calculated by the simple fluid Material
# Pressure unit is chosen to be MPa
# Time unit is chosen to be years
# Temperature unit is chosen to be Celsius
# Pressure 10 MPa
# Temperature = 26.85 C
# Density should equal 1500*exp(1E7/1E9-2E-4*300)=1426.844 kg/m^3
# Viscosity should equal 3.49E-17 MPa.yr
# Energy density should equal 4000 * 300 = 1.2E6 J/kg
# Specific enthalpy should equal 4000 * 300 + 10e6 / 1426.844 = 1.207008E6 J/kg

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 2.0E-4
    cv = 4000.0
    cp = 5000.0
    bulk_modulus = 1.0E9
    thermal_conductivity = 1.0
    viscosity = 1.1E-3
    density0 = 1500.0
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp T'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[Variables]
  [pp]
    initial_condition = 10
  []
  [T]
    initial_condition = 26.85
  []
[]

[Kernels]
  [dummy_p]
    type = Diffusion
    variable = pp
  []
  [dummy_T]
    type = Diffusion
    variable = T
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = T
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    temperature_unit = Celsius
    pressure_unit = MPa
    time_unit = years
    fp = the_simple_fluid
    phase = 0
  []
[]

[Postprocessors]
  [pressure]
    type = ElementIntegralVariablePostprocessor
    variable = pp
  []
  [temperature]
    type = ElementIntegralVariablePostprocessor
    variable = T
  []
  [density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
  []
  [viscosity]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity_qp0'
  []
  [internal_energy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_internal_energy_qp0'
  []
  [enthalpy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_enthalpy_qp0'
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
