# Test the density and viscosity calculated by the water Material
# Region 1 density
# Pressure 80 MPa
# Temperature 300K (26.85C)
# Water density should equal 1.0 / 0.971180894e-3 = 1029.7 kg/m^3 (IAPWS IF97)
# Water viscosity should equal 0.00085327 Pa.s (NIST webbook)
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
    initial_condition = 80e6
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
    initial_condition = 300.0
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
  [water]
    type = PorousFlowSingleComponentFluid
    temperature_unit = Kelvin
    fp = water
    phase = 0
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
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
  file_base = h2o
  csv = true
[]
