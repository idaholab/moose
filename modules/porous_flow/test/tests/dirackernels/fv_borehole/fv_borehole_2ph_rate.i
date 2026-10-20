# FVPorousFlowPeacemanBorehole with control = rate: water and a CO2-like gas (immiscible), with
# one DiracKernel per phase and component.  The reservoir is initially full of water and gas is
# injected at 1 kg/s (the injecting borehole points use injection_phase = 1).  The production
# test sets an initial gas saturation of 0.3 and a production rate of 1 kg/s, which produces both
# phases.  The results agree with MRST: see the FVPorousFlowPeacemanBorehole documentation
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 6
  xmin = -10
  xmax = 10
  ymin = -10
  ymax = 10
  zmin = -15
  zmax = 15
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [ppwater]
    type = MooseVariableFVReal
  []
  [sgas]
    type = MooseVariableFVReal
    initial_condition = 0
  []
  [bhp]
    # bottomhole pressure, solved fully coupled with ppwater and sgas
    family = SCALAR
    order = FIRST
    # starting guess: hydrostatic pressure at the bottom point
    initial_condition = 1.0025E7
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
    type = MooseVariableFVReal
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    type = MooseVariableFVReal
    initial_condition = 0
  []
[]

[ICs]
  [ppwater]
    type = FunctionIC
    variable = ppwater
    function = '1E7 - 1E4 * z'
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = ppwater
  []
  [flux0]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 0
    variable = ppwater
    gravity = '0 0 -10'
  []
  [mass1]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  []
  [flux1]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
    gravity = '0 0 -10'
  []
[]

[DiracKernels]
  # One DiracKernel per fluid phase and component, as PorousFlowPeacemanBorehole.  The mass
  # fractions of water in the gas and of gas in the water are zero, so those two are not needed
  [bh_water]
    type = FVPorousFlowPeacemanBorehole
    variable = ppwater
    fluid_phase = 0
    mass_fraction_component = 0
    control = rate
    bottom_p = bhp
    rate = rate
    point_file = fv_borehole.bh
    unit_weight = '0 0 -1E4'
    character = 1
    use_mobility = true
    allow_crossflow = true
    injection_phase = 1
  []
  [bh_gas]
    type = FVPorousFlowPeacemanBorehole
    variable = sgas
    fluid_phase = 1
    mass_fraction_component = 1
    control = rate
    bottom_p = bhp
    rate = rate
    point_file = fv_borehole.bh
    unit_weight = '0 0 -1E4'
    character = 1
    use_mobility = true
    allow_crossflow = true
    injection_phase = 1
  []
[]

[ScalarKernels]
  [bhp]
    # The bhp equation (rate - sum of the borehole fluxes = 0) is assembled by the borehole
    # DiracKernels: this only tells MOOSE that bhp has an equation
    type = NullScalarKernel
    variable = bhp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater sgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
[]

[FluidProperties]
  [water]
    type = SimpleFluidProperties
    bulk_modulus = 2E9
    viscosity = 1E-3
    density0 = 1000
    thermal_expansion = 0
  []
  [gas]
    # CO2-like: density = 364 * exp(p / 2E7), about 600 kg/m^3 at 10 MPa
    type = SimpleFluidProperties
    bulk_modulus = 2E7
    viscosity = 5E-5
    density0 = 364
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow2PhasePS
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [massfrac]
    type = ADPorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [water]
    type = ADPorousFlowSingleComponentFluid
    fp = water
    phase = 0
  []
  [gas]
    type = ADPorousFlowSingleComponentFluid
    fp = gas
    phase = 1
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1E-12 0 0 0 1E-12 0 0 0 1E-12'
  []
  [relperm_water]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm_gas]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
  []
[]

[Functions]
  [rate]
    # kg/s, positive for production
    type = ParsedFunction
    expression = '-1'
  []
[]

[Postprocessors]
  [p_bot]
    type = ScalarVariable
    variable = bhp
  []
  [target_rate]
    type = FunctionValuePostprocessor
    function = rate
  []
  # Mass rates leaving the porous medium, from the change in the fluid mass of each component
  [water_mass]
    type = FVPorousFlowFluidMass
    fluid_component = 0
  []
  [water_mass_old]
    type = FVPorousFlowFluidMass
    fluid_component = 0
    execute_on = 'timestep_begin'
    outputs = none
  []
  [gas_mass]
    type = FVPorousFlowFluidMass
    fluid_component = 1
  []
  [gas_mass_old]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    execute_on = 'timestep_begin'
    outputs = none
  []
  [dt]
    type = TimestepSize
    outputs = none
  []
  [water_rate]
    type = ParsedPostprocessor
    expression = '(water_mass_old - water_mass) / dt'
    pp_names = 'water_mass water_mass_old dt'
  []
  [gas_rate]
    type = ParsedPostprocessor
    expression = '(gas_mass_old - gas_mass) / dt'
    pp_names = 'gas_mass gas_mass_old dt'
  []
  [bh_rate]
    type = ParsedPostprocessor
    expression = 'water_rate + gas_rate'
    pp_names = 'water_rate gas_rate'
  []
  # Gas saturation in the cells of the top and bottom borehole points
  [sgas_top]
    type = PointValue
    variable = sgas
    point = '2.5 2.5 7.5'
  []
  [sgas_bot]
    type = PointValue
    variable = sgas
    point = '2.5 2.5 -2.5'
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
  solve_type = NEWTON
  end_time = 100
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100'
  []
  nl_abs_tol = 1E-10
  nl_rel_tol = 1E-12
[]

[Outputs]
  csv = true
[]
