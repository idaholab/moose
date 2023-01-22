# Two phase density-driven convection of dissolved CO2 in brine
#
# The model starts with CO2 in the liquid phase only.  The CO2 diffuses into the brine.
# As the density of the CO2-saturated brine is greater
# than the unsaturated brine, a gravitational instability arises and density-driven
# convection of CO2-rich fingers descend into the unsaturated brine.
#
# The instability is seeded by a random perturbation to the porosity field.
# Mesh adaptivity is used to refine the mesh as the fingers form.
#
# Note: this model is computationally expensive, so should be run with multiple cores.

[GlobalParams]
  PorousFlowDictator = 'dictator'
  gravity = '0 -9.81 0'
[]

[Adaptivity]
  max_h_level = 2
  marker = marker
  initial_marker = initial
  initial_steps = 2
  [Indicators]
    [indicator]
      type = GradientJumpIndicator
      variable = zi
    []
  []
  [Markers]
    [marker]
      type = ErrorFractionMarker
      indicator = indicator
      refine = 0.8
    []
    [initial]
      type = BoxMarker
      bottom_left = '0 1.95 0'
      top_right = '2 2 0'
      inside = REFINE
      outside = DO_NOTHING
    []
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  ymin = 1.5
  ymax = 2
  xmax = 2
  ny = 20
  nx = 40
  bias_y = 0.95
[]

[AuxVariables]
  [xnacl]
    initial_condition = 0.01
  []
  [saturation_gas]
    order = FIRST
    family = MONOMIAL
  []
  [xco2l]
    order = FIRST
    family = MONOMIAL
  []
  [density_liquid]
    order = FIRST
    family = MONOMIAL
  []
  [porosity]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [saturation_gas]
    type = PorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = 'timestep_end'
  []
  [xco2l]
    type = PorousFlowPropertyAux
    variable = xco2l
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = 'timestep_end'
  []
  [density_liquid]
    type = PorousFlowPropertyAux
    variable = density_liquid
    property = density
    phase = 0
    execute_on = 'timestep_end'
  []
[]

[Variables]
  [pgas]
  []
  [zi]
    scaling = 1e4
  []
[]

[ICs]
  [pressure]
    type = FunctionIC
    function = 10e6-9.81*1000*y
    variable = pgas
  []
  [zi]
    type = ConstantIC
    value = 0
    variable = zi
  []
  [porosity]
    type = RandomIC
    variable = porosity
    min = 0.25
    max = 0.275
    seed = 0
  []
[]

[BCs]
  [top]
    type = DirichletBC
    value = 0.04
    variable = zi
    boundary = top
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pgas
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pgas
  []
  [diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = pgas
    disp_long = '0 0'
    disp_trans = '0 0'
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = zi
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = zi
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = zi
    disp_long = '0 0'
    disp_trans = '0 0'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas zi'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
  [fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2
    capillary_pressure = pc
  []
[]

[FluidProperties]
  [co2sw]
    type = CO2FluidProperties
  []
  [co2]
    type = TabulatedFluidProperties
    fp = co2sw
  []
  [brine]
    type = BrineFluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = '45'
  []
  [brineco2]
    type = PorousFlowFluidState
    gas_porepressure = 'pgas'
    z = 'zi'
    temperature_unit = Celsius
    xnacl = 'xnacl'
    capillary_pressure = pc
    fluid_state = fs
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = porosity
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-11 0 0 0 1e-11 0 0 0 1e-11'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    phase = 0
    n = 2
    s_res = 0.1
    sum_s_res = 0.2
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    phase = 1
    n = 2
    s_res = 0.1
    sum_s_res = 0.2
  []
  [diffusivity]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '2e-9 2e-9 2e-9 2e-9'
    tortuosity = '1 1'
  []
[]

[Preconditioning]
  active = basic
  [mumps_is_best_for_parallel_jobs]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
  [basic]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2             '
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 1e6
  nl_max_its = 25
  l_max_its = 100
  dtmax = 1e4
  nl_abs_tol = 1e-6
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 100
    growth_factor = 2
    cutback_factor = 0.5
  []
[]

[Functions]
  [flux]
    type = ParsedFunction
    symbol_values = 'delta_xco2 dt'
    symbol_names = 'dx dt'
    expression = 'dx/dt'
  []
[]

[Postprocessors]
  [total_co2_in_gas]
    type = PorousFlowFluidMass
    phase = 1
    fluid_component = 1
  []
  [total_co2_in_liquid]
    type = PorousFlowFluidMass
    phase = 0
    fluid_component = 1
  []
  [numdofs]
    type = NumDOFs
  []
  [delta_xco2]
    type = ChangeOverTimePostprocessor
    postprocessor = total_co2_in_liquid
  []
  [dt]
    type = TimestepSize
  []
  [flux]
    type = FunctionValuePostprocessor
    function = flux
  []
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  exodus = true
  csv = true
[]
