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
    porous_flow_vars = 'ADpp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[Variables]
  [ADpp]
    initial_condition = 20e6
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FVKernels]
  [dummy]
    type = FVDiffusion
    variable = ADpp
    coeff = 1
  []
[]

[AuxVariables]
  [ADtemp]
    initial_condition = 50
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [ADxnacl]
    initial_condition = 0.1047
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[FluidProperties]
  [brine]
    type = BrineFluidProperties
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
    temperature = ADtemp
  []
  [ppss]
    type = ADPorousFlow1PhaseFullySaturated
    porepressure = ADpp
  []
  [brine]
    type = ADPorousFlowMultiComponentFluid
    temperature_unit = Celsius
    x = ADxnacl
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
    variable = ADpp
  []
  [temperature]
    type = ElementIntegralVariablePostprocessor
    variable = ADtemp
  []
  [xnacl]
    type = ElementIntegralVariablePostprocessor
    variable = ADxnacl
  []
  [density]
    type = ADElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
  []
  [viscosity]
    type = ADElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity_qp0'
  []
  [enthalpy]
    type = ADElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_enthalpy_qp0'
  []
  [internal_energy]
    type = ADElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_internal_energy_qp0'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = brine1
  csv = true
[]
