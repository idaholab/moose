# Test the density and viscosity calculated by the water Material
# Region 2 density
# Pressure 3.5 kPa
# Temperature 300K (26.85C)
# Water density should equal 1.0 / 0.394913866e2 = 0.025323 kg/m^3 (IAPWS IF97)
# Water viscosity should equal 9.9196e-06 Pa.s (NIST webbook)
# Results are within expected accuracy

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Variables]
  [./pp]
    initial_condition = 3.5e3
  [../]
[]

[Kernels]
  [./dummy]
    type = Diffusion
    variable = pp
  [../]
[]

[AuxVariables]
  [./temp]
    initial_condition = 26.85
  [../]
[]

[Materials]
  [./ppss]
    type = PorousFlowMaterial1PhaseP
    porepressure = pp
    temperature = 'temp'
  [../]
  [./dens0]
    type = PorousFlowMaterialWater
    outputs = all
    phase = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Postprocessors]
  [./pressure]
    type = ElementIntegralVariablePostprocessor
    variable = pp
  [../]
  [./temperature]
    type = ElementIntegralVariablePostprocessor
    variable = temp
  [../]
  [./density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density0'
  [../]
  [./viscosity]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity0'
  [../]
  [./ddensity_dp]
    type = ElementIntegralMaterialProperty
    mat_prop = 'dPorousFlow_fluid_phase_density0/dpressure_variable_dummy'
  [../]
  [./ddensity_dt]
    type = ElementIntegralMaterialProperty
    mat_prop = 'dPorousFlow_fluid_phase_density0/dtemperature_variable_dummy'
  [../]
  [./dviscosity_dt]
    type = ElementIntegralMaterialProperty
    mat_prop = 'dPorousFlow_viscosity0/dtemperature_variable_dummy'
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = h2o2
  csv = true
[]
