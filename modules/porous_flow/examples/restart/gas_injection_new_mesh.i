# Using the results from the equilibrium run to provide the initial condition for
# porepressure, we now inject a gas phase into the brine-saturated reservoir. In this
# example, the mesh is not identical to the mesh used in gravityeq.i. Rather, it is
# generated so that it is more refined near the injection boundary and at the top of
# the model, as that is where the gas plume will be present.
#
# To use the hydrostatic pressure calculated using the gravity equilibrium run as the initial
# condition for the pressure, a SolutionUserObject is used, along with a SolutionFunction to
# interpolate the pressure from the gravity equilibrium run to the initial condition for liqiud
# porepressure in this example.
#
# Even though the gravity equilibrium is established using a 2D mesh, in this example,
# we use a mesh shifted 0.1 m to the right and rotate it about the Y axis to make a 2D radial
# model.
#
# Methane injection takes place over the surface of the hole created by rotating the mesh,
# and hence the injection area is 2 pi r h. We can calculate this using an AreaPostprocessor,
# and then use this in a ParsedFunction to calculate the injection rate so that 10 kg/s of
# methane is injected.
#
# Note: as this example uses the results from a previous simulation, gravityeq.i MUST be
# run before running this input file.

[Mesh]
  type = GeneratedMesh
  dim = 2
  ny = 25
  nx = 50
  ymax = 100
  xmin = 0.1
  xmax = 5000
  bias_x = 1.05
  bias_y = 0.95
[]

[Problem]
  coord_type = RZ
  rz_coord_axis = Y
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 -9.81 0'
  temperature_unit = Celsius
[]

[Variables]
  [pp_liq]
  []
  [sat_gas]
    initial_condition = 0
  []
[]

[ICs]
  [ppliq_ic]
    type = FunctionIC
    variable = pp_liq
    function = ppliq_ic
  []
[]

[AuxVariables]
  [temperature]
    initial_condition = 50
  []
  [xnacl]
    initial_condition = 0.1
  []
  [brine_density]
    family = MONOMIAL
    order = CONSTANT
  []
  [methane_density]
    family = MONOMIAL
    order = CONSTANT
  []
  [massfrac_ph0_sp0]
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    initial_condition = 0
  []
  [pp_gas]
    family = MONOMIAL
    order = CONSTANT
  []
  [sat_liq]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    variable = pp_liq
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    variable = pp_liq
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    variable = sat_gas
    fluid_component = 1
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    variable = sat_gas
    fluid_component = 1
  []
[]

[AuxKernels]
  [brine_density]
    type = PorousFlowPropertyAux
    property = density
    variable = brine_density
    execute_on = 'initial timestep_end'
  []
  [methane_density]
    type = PorousFlowPropertyAux
    property = density
    variable = methane_density
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [pp_gas]
    type = PorousFlowPropertyAux
    property = pressure
    phase = 1
    variable = pp_gas
    execute_on = 'initial timestep_end'
  []
  [sat_liq]
    type = PorousFlowPropertyAux
    property = saturation
    variable = sat_liq
    execute_on = 'initial timestep_end'
  []
[]

[BCs]
  [gas_injection]
    type = PorousFlowSink
    boundary = left
    variable = sat_gas
    flux_function = injection_rate
    fluid_phase = 1
  []
  [brine_out]
    type = PorousFlowPiecewiseLinearSink
    boundary = right
    variable = pp_liq
    multipliers = '0 1e9'
    pt_vals = '0 1e9'
    fluid_phase = 0
    flux_function = 1e-6
    use_mobility = true
    use_relperm = true
    mass_fraction_component = 0
  []
[]

[Functions]
  [injection_rate]
    type = ParsedFunction
    symbol_values = injection_area
    symbol_names = area
    expression = '-1/area'
  []
  [ppliq_ic]
    type = SolutionFunction
    solution = soln
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp_liq sat_gas'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1e-5
    m = 0.5
    sat_lr = 0.2
    pc_max = 1e7
  []
  [soln]
    type = SolutionUserObject
    mesh = gravityeq_out.e
    system_variables = porepressure
  []
[]

[FluidProperties]
  [brine]
    type = BrineFluidProperties
  []
  [methane]
    type = MethaneFluidProperties
  []
  [methane_tab]
    type = TabulatedFluidProperties
    fp = methane
    save_file = false
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temperature
  []
  [ps]
    type = PorousFlow2PhasePS
    phase0_porepressure = pp_liq
    phase1_saturation = sat_gas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [brine]
    type = PorousFlowBrine
    compute_enthalpy = false
    compute_internal_energy = false
    xnacl = xnacl
    phase = 0
  []
  [methane]
    type = PorousFlowSingleComponentFluid
    compute_enthalpy = false
    compute_internal_energy = false
    fp = methane_tab
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-13 0 0 0 5e-14 0  0 0 1e-13'
  []
  [relperm_liq]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    s_res = 0.2
    sum_s_res = 0.3
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
    s_res = 0.1
    sum_s_res = 0.3
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type'
    petsc_options_value = ' asm      lu           NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1e8
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-06
  nl_max_its = 20
  dtmax = 1e6
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e1
    growth_factor = 1.5
  []
[]

[Postprocessors]
  [mass_ph0]
    type = PorousFlowFluidMass
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [mass_ph1]
    type = PorousFlowFluidMass
    fluid_component = 1
    execute_on = 'initial timestep_end'
  []
  [injection_area]
    type = AreaPostprocessor
    boundary = left
    execute_on = initial
  []
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
  perf_graph = true
[]
