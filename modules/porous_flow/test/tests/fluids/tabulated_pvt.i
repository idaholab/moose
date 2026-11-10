# Test the properties calculated by TabulatedPVTFluidProperties
# Table (PVDO style):
#   p (Pa)   B (rm3/sm3)   mu (Pa.s)
#   2.0E6    1.062         1.04E-3
#   1.0E7    1.050         1.10E-3
#   2.0E7    1.038         1.17E-3
#   3.0E7    1.028         1.24E-3
# density_sc = 850 kg/m^3, cv = 2000 J/kg/K
# 1/B and 1/(B mu) are interpolated linearly in pressure.
# At p = 15 MPa, T = 350 K:
#   1/B      = (1/1.050 + 1/1.038) / 2 = 0.957886044...
#   density  = 850 / B = 814.2031379 kg/m^3
#   1/(B mu) = (1/(1.050*1.10E-3) + 1/(1.038*1.17E-3)) / 2 = 844.6006...
#   mu       = (1/B) / (1/(B mu)) = 1.134122E-3 Pa.s
#   e        = cv * T = 7.0E5 J/kg
#   h        = e + p / density = 7.184229E5 J/kg

[FluidProperties]
  [oil]
    type = TabulatedPVTFluidProperties
    density_sc = 850
    pressure = '2.0E6 1.0E7 2.0E7 3.0E7'
    formation_volume_factor = '1.062 1.050 1.038 1.028'
    viscosity = '1.04E-3 1.10E-3 1.17E-3 1.24E-3'
    cv = 2000
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
    initial_condition = 15e6
  []
  [T]
    initial_condition = 350.0
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
  [oil]
    type = PorousFlowSingleComponentFluid
    temperature_unit = Kelvin
    fp = oil
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
  file_base = tabulated_pvt
  csv = true
[]
