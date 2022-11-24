# Using flux-limited TVD advection ala Kuzmin and Turek, employing PorousFlow Kernels and UserObjects, with superbee flux-limiter
# 3D version
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  xmin = 0
  xmax = 1
  ny = 4
  ymin = 0
  ymax = 0.5
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
    function = '1 - x'
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
    value = 1
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
  active = basic
  [basic]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
  [preferred_but_might_not_be_installed]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
[]

[VectorPostprocessors]
  [tracer]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0.5 0'
    num_points = 11
    sort_by = x
    variable = tracer
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 6
  dt = 6E-2
  nl_abs_tol = 1E-8
  timestep_tolerance = 1E-3
[]

[Outputs]
  [out]
    type = CSV
    execute_on = final
  []
[]
