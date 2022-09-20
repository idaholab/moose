# Checking that the mass postprocessor throws the correct error when a given phase index
# is too large

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
  [sat]
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    initial_condition = 0
  []
[]

[ICs]
  [pinit]
    type = ConstantIC
    value = 1
    variable = pp
  []
  [satinit]
    type = FunctionIC
    function = 1-x
    variable = sat
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sat
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp sat'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 1
    thermal_expansion = 0
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 0.1
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = pp
    phase1_saturation = sat
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
[]

[Postprocessors]
  [comp1_total_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 2
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]
