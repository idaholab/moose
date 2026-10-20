# FVPorousFlowPeacemanBorehole with control = rate: single phase, three borehole points in
# different cells.  The borehole produces 1 kg/s until t = 50 s, then injects 0.5 kg/s, and the
# bottomhole pressure (the scalar variable bhp) is computed.  The results agree with MRST: see the
# FVPorousFlowPeacemanBorehole documentation
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
  [pp]
    type = MooseVariableFVReal
  []
  [bhp]
    # bottomhole pressure, solved fully coupled with pp
    family = SCALAR
    order = FIRST
    # starting guess: hydrostatic pressure at the bottom point
    initial_condition = 1.0025E7
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = '1E7 - 1E4 * z'
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux0]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '0 0 -10'
  []
[]

[DiracKernels]
  [bh]
    type = FVPorousFlowPeacemanBorehole
    variable = pp
    control = rate
    bottom_p = bhp
    rate = rate
    point_file = fv_borehole.bh
    unit_weight = '0 0 -1E4'
    character = 1
    use_mobility = true
    allow_crossflow = true
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
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E9
    viscosity = 1E-3
    density0 = 1000
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
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
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1E-12 0 0 0 1E-12 0 0 0 1E-12'
  []
  [relperm]
    type = ADPorousFlowRelativePermeabilityConst
    phase = 0
  []
[]

[Functions]
  [rate]
    type = ParsedFunction
    expression = 'if(t <= 50, 1, -0.5)'
  []
[]

[Postprocessors]
  [target_rate]
    type = FunctionValuePostprocessor
    function = rate
  []
  [p_bot]
    type = ScalarVariable
    variable = bhp
  []
  [mass]
    type = FVPorousFlowFluidMass
  []
  [bh_rate]
    # mass rate leaving the porous medium, from the change in fluid mass
    type = ParsedPostprocessor
    expression = '(mass_old - mass) / dt'
    pp_names = 'mass mass_old dt'
  []
  [mass_old]
    type = FVPorousFlowFluidMass
    execute_on = 'timestep_begin'
    outputs = none
  []
  [dt]
    type = TimestepSize
    outputs = none
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
