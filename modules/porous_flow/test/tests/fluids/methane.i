# Test MethaneFluidProperties
# Reference data from Irvine Jr, T. F. and Liley, P. E. (1984) Steam and
# Gas Tables with Computer Equations
#
# For temperature = 350K, the fluid properties should be:
# density = 55.13 kg/m^3
# viscosity = 0.01276 mPa.s
# h = 708.5 kJ/kg

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
    initial_condition = 10e6
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
    initial_condition = 350.0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 'temp'
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [methane]
    type = PorousFlowSingleComponentFluid
    temperature_unit = Kelvin
    fp = methane
    phase = 0
  []
[]

[FluidProperties]
  [methane]
    type = MethaneFluidProperties
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
  [enthalpy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_enthalpy_qp0'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = methane
  csv = true
[]
