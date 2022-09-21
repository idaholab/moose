# Checking that the mass postprocessor correctly calculates the mass
# of each component in each phase, as well as the total mass of each
# component in all phases. Also tests that optional saturation threshold
# gives the correct mass
# 2phase, 2component, constant porosity
# saturation_threshold set to 0.6 for phase 1

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
    type = PorousFlowCapillaryPressureConst
    pc = 0
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
  [comp0_phase0_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    phase = 0
  []
  [comp0_phase1_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    phase = 1
  []
  [comp0_total_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
  []
  [comp1_phase0_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 0
  []
  [comp1_phase1_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 1
  []
  [comp1_total_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
  []
  [comp1_phase1_threshold_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 1
    saturation_threshold = 0.6
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  nl_abs_tol = 1e-16
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = mass06
  csv = true
[]
