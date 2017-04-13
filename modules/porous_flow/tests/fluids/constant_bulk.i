# Test the density calculated by the constant bulk Material
# Pressure 10 MPa
# Bulk modulus 2e8
# Density0 = 1000
# Density should equal 1051.27 kg/m^3
# dDensity_dPressure should equal 5.256355e-06

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

[Materials]
  [./temperature]
    type = PorousFlowTemperature
  [../]
  [./ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    bulk_modulus = 2e8
    density_P0 = 1000
    phase = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Postprocessors]
  [./density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
  [../]
  [./ddensity_dp]
    type = ElementIntegralMaterialProperty
    mat_prop = 'dPorousFlow_fluid_phase_density_qp0/dpressure_variable_dummy'
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = constant_bulk
  csv = true
[]
