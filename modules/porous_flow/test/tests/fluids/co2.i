# Test the density and viscosity calculated by the simple CO2 Material
# Pressure 5 MPa
# Temperature 50C
# These conditions correspond to the gas phase
# CO2 density should equal 104 kg/m^3 (NIST webbook)
# CO2 viscosity should equal 0.000017345 Pa.s (NIST webbook)
# Results are within expected accuracy

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
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[Variables]
  [pp]
    initial_condition = 5e6
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = pp
  []
[]

[AuxVariables]
  [temp]
    initial_condition = 50
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [co2]
    type = PorousFlowSingleComponentFluid
    temperature_unit = Celsius
    fp = co2
    phase = 0
  []
[]

[FluidProperties]
  [co2]
    type = CO2FluidProperties
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Postprocessors]
  [pressure]
    type = ElementIntegralVariablePostprocessor
    variable = pp
  []
  [temperature]
    type = ElementIntegralVariablePostprocessor
    variable = temp
  []
  [density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
  []
  [viscosity]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity_qp0'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = co2
  csv = true
[]
