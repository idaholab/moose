# Test the density calculated by the ideal gas Material
# Pressure 10 MPa
# molar_mass = 5e-3
# Density should equal 18.60937 kg/m^3
# dDensity_dPressure should equal 1.860937e-06
# dDensity_dTemperature should equal -5.758741e-02

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
    initial_condition = 10e6
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
    initial_condition = 50.0
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    temperature = temp
  [../]
  [./ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./dens0]
    type = PorousFlowIdealGas
    temperature_unit = Celsius
    molar_mass = 5e-3
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
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
  [../]
  [./ddensity_dp]
    type = ElementIntegralMaterialProperty
    mat_prop = 'dPorousFlow_fluid_phase_density_qp0/dpressure_variable_dummy'
  [../]
  [./ddensity_dt]
    type = ElementIntegralMaterialProperty
    mat_prop = 'dPorousFlow_fluid_phase_density_qp0/dtemperature_variable_dummy'
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = ideal_gas
  csv = true
[]
