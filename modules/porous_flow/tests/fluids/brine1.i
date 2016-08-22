# Test the density and viscosity calculated by the brine Material
# Pressure 20 MPa
# Temperature 50C
# xnacl = 0.1047 (equivalent to 2.0 molality)
# Brine density should equal 1100.87 kg/m^3 (Pitzer et al, 1984)
# Brine viscosity should equal 0.0006777 Pa.s (Phillips et al, 1980)
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
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Variables]
  [./pp]
    initial_condition = 20e6
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
    initial_condition = 50
  [../]
  [./xnacl]
    initial_condition = 0.1047
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    temperature = temp
  [../]
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./dens0]
    type = PorousFlowBrine
    xnacl = 0.1047
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
  [./xnacl]
    type = ElementIntegralVariablePostprocessor
    variable = xnacl
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
  file_base = brine1
  csv = true
[]
