# Checks that ADPorousFlowPorosity sets the porosity to porosity_min when the
# computed porosity is less than porosity_min.
# With thermal = true, porosity = 1 + (0.1 - 1) * exp(0.1 * (T - 0)) = -1.45
# for T = 10, so the porosity should be porosity_min = 0.05.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    type = MooseVariableFVReal
    initial_condition = 1
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [porosity]
    type = ADPorousFlowPropertyAux
    property = porosity
    variable = porosity
    execute_on = 'initial timestep_end'
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
    temperature = 10
  []
  [ppss]
    type = ADPorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac]
    type = ADPorousFlowMassFraction
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = ADPorousFlowPorosity
    thermal = true
    porosity_zero = 0.1
    thermal_expansion_coeff = 0.1
    ensure_positive = false
    porosity_min = 0.05
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    variable = pp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Postprocessors]
  [porosity]
    type = ElementAverageValue
    variable = porosity
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  csv = true
[]
