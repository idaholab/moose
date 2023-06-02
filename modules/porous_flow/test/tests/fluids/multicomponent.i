# Test the density and viscosity calculated by the brine material using PorousFlowMultiComponentFluid
# Pressure 20 MPa
# Temperature 50C
# xnacl = 0.1047 (equivalent to 2.0 molality)

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
    initial_condition = 20e6
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
  [xnacl]
    initial_condition = 0.1047
  []
[]

[FluidProperties]
    [brine]
        type = BrineFluidProperties
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
  [brine]
    type = PorousFlowMultiComponentFluid
    temperature_unit = Celsius
    x = xnacl
    phase = 0
    fp = brine
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
  [xnacl]
    type = ElementIntegralVariablePostprocessor
    variable = xnacl
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
  [internal_energy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_internal_energy_qp0'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = brine1
  csv = true
[]
