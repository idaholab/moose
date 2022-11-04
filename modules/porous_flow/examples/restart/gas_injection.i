# Using the results from the equilibrium run to provide the initial condition for
# porepressure, we now inject a gas phase into the brine-saturated reservoir. In this
# example, where the mesh used is identical to the mesh used in gravityeq.i, we can use
# the basic restart capability by simply setting the initial condition for porepressure
# using the results from gravityeq.i.
#
# Even though the gravity equilibrium is established using a 2D mesh, in this example,
# we shift the mesh 0.1 m to the right and rotate it about the Y axis to make a 2D radial
# model.
#
# Methane injection takes place over the surface of the hole created by rotating the mesh,
# and hence the injection area is 2 pi r h. We can calculate this using an AreaPostprocessor,
# and then use this in a ParsedFunction to calculate the injection rate so that 10 kg/s of
# methane is injected.
#
# Results can be improved by uniformly refining the initial mesh.
#
# Note: as this example uses the results from a previous simulation, gravityeq.i MUST be
# run before running this input file.

[Mesh]
  uniform_refine = 1
  [file]
    type = FileMeshGenerator
    file = gravityeq_out.e
  []
  [translate]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '0.1 0 0'
    input = file
  []
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
    initial_from_file_var = porepressure
  []
  [sat_gas]
    initial_condition = 0
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
  []
[]

[Functions]
  [injection_rate]
    type = ParsedFunction
    symbol_values = injection_area
    symbol_names = area
    expression = '-10/area'
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
    permeability = '1e-13 0 0 0 1e-13 0  0 0 1e-13'
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
  checkpoint = true
[]
