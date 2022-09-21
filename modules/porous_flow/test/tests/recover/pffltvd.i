# Tests that PorousFlow can successfully recover using a checkpoint file.
# This test contains stateful material properties, adaptivity, integrated
# boundary conditions with nodal-sized materials, and TVD flux limiting.
#
# This test file is run three times:
# 1) The full input file is run to completion
# 2) The input file is run for half the time and checkpointing is included
# 3) The input file is run in recovery using the checkpoint data
#
# The final output of test 3 is compared to the final output of test 1 to verify
# that recovery was successful.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[Adaptivity]
  initial_steps = 1
  initial_marker = tracer_marker
  marker = tracer_marker
  max_h_level = 1
  [Markers]
    [tracer_marker]
      type = ValueRangeMarker
      variable = tracer
      lower_bound = 0.02
      upper_bound = 0.98
    []
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [porepressure]
  []
  [tracer]
  []
[]

[ICs]
  [porepressure]
    type = FunctionIC
    variable = porepressure
    function = '2 - x'
  []
  [tracer]
    type = FunctionIC
    variable = tracer
    function = 'if(x<0.1,0,if(x>0.3,0,1))'
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = tracer
  []
  [flux0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = tracer
    advective_flux_calculator = advective_flux_calculator_0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = porepressure
  []
  [flux1]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = porepressure
    advective_flux_calculator = advective_flux_calculator_1
  []
[]

[BCs]
  [constant_injection_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 2
    boundary = left
  []
  [no_tracer_on_left]
    type = DirichletBC
    variable = tracer
    value = 0
    boundary = left
  []
  [remove_component_1]
    type = PorousFlowPiecewiseLinearSink
    variable = porepressure
    boundary = right
    fluid_phase = 0
    pt_vals = '0 1E3'
    multipliers = '0 1E3'
    mass_fraction_component = 1
    use_mobility = true
    flux_function = 1E3
  []
  [remove_component_0]
    type = PorousFlowPiecewiseLinearSink
    variable = tracer
    boundary = right
    fluid_phase = 0
    pt_vals = '0 1E3'
    multipliers = '0 1E3'
    mass_fraction_component = 0
    use_mobility = true
    flux_function = 1E3
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E9
    thermal_expansion = 0
    viscosity = 1.0
    density0 = 1000.0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'porepressure tracer'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
  []
  [advective_flux_calculator_0]
    type = PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent
    flux_limiter_type = superbee
    fluid_component = 0
  []
  [advective_flux_calculator_1]
    type = PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent
    flux_limiter_type = superbee
    fluid_component = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = porepressure
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = tracer
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = the_simple_fluid
    phase = 0
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-2 0 0   0 1E-2 0   0 0 1E-2'
  []
[]

[Preconditioning]
  [basic]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
[]

[VectorPostprocessors]
  [tracer]
    type = NodalValueSampler
    sort_by = x
    variable = tracer
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 0.2
  dt = 0.05
[]

[Outputs]
  csv = true
[]
