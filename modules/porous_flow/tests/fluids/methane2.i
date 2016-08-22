# Test the density and viscosity calculated by the methane Material
# Pressure 2 MPa
# Temperature 150C
# Water density should equal 9.1744 kg/m^3 (NIST webbook)
# Water viscosity should equal 1.5104e-05 Pa.s (NIST webbook)

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
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
    initial_condition = 2e6
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
    initial_condition = 150
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    temperature = 'temp'
  [../]
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./methane]
    type = PorousFlowMethane
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
  file_base = methane2
  csv = true
[]
