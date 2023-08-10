################################################################################
## Molten Salt Fast Reactor - Euratom EVOL + Rosatom MARS Design              ##
## Pronghorn Sub-Application input file                                       ##
## Relaxation towards Steady state 3D thermal hydraulics model                ##
################################################################################
# If using or referring to this model, please cite as explained in
# https://mooseframework.inl.gov/virtual_test_bed/citing.html

# The flow in this simulation should be initialized with a previous flow
# solution (isothermal, heated or multiphysics) OR using a viscosity rampdown
# An isothermal solution can be generated using 'run_ns_initial.i', and is
# saved in 'restart/run_ns_initial_restart.e'
# A heated solution, with a flat power distribution, can be generated with
# this script 'run_ns.i', and is saved in 'restart/run_ns_restart.e'
# A coupled neutronics-coarse TH solution can be generated with
# 'run_neutronics.i', saved in 'restart/run_neutronics_ns_restart.e'

# Material properties
#rho = 4284  # density [kg / m^3]  (@1000K)
rho = 3197.49 # density [kg / m^3]  (@1000K)
cp = 1594 # specific heat capacity [J / kg / K]
drho_dT = 0.882 # derivative of density w.r.t. temperature [kg / m^3 / K]
mu = 0.0166 # viscosity [Pa s]
k = 1.7 # thermal conductivity [W / m / K]
# https://www.researchgate.net/publication/337161399_Development_of_a_control-\
# oriented_power_plant_simulator_for_the_molten_salt_fast_reactor/fulltext/5dc9\
# 5c9da6fdcc57503eec39/Development-of-a-control-oriented-power-plant-simulator-\
# for-the-molten-salt-fast-reactor.pdf
von_karman_const = 0.41

# Turbulent properties
Pr_t = 0.9 # turbulent Prandtl number
Sc_t = 1 # turbulent Schmidt number

# Derived material properties
alpha = '${fparse drho_dT / rho}' # thermal expansion coefficient

# Operating parameters
T_HX = 873.15 # heat exchanger temperature [K]

# Mass flow rate tuning, for heat exchanger pressure and temperature drop
friction = 4e3 # [kg / m^4]
pump_force = -20000. # [N / m^3]

# Delayed neutron precursor parameters. Lambda values are decay constants in
# [1 / s]. Beta values are production fractions.
lambda1_m = -0.0133104
lambda2_m = -0.0305427
lambda3_m = -0.115179
lambda4_m = -0.301152
lambda5_m = -0.879376
lambda6_m = -2.91303
beta1 = 8.42817e-05
beta2 = 0.000684616
beta3 = 0.000479796
beta4 = 0.00103883
beta5 = 0.000549185
beta6 = 0.000184087

[GlobalParams]
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
[]

################################################################################
# GEOMETRY
################################################################################

[Mesh]
  coord_type = 'RZ'
  rz_coord_axis = Y
  allow_renumbering = false

  [restart]
    type = FileMeshGenerator
    use_for_exodus_restart = true
    # Depending on the file chosen, the initialization of variables should be
    # adjusted. The following variables can be initalized:
    # - vel_x, vel_y, p from isothermal simulation
    file = 'run_ns_initial_test_restart.e'
    #file = 'restart/run_ns_initial_restart.e'
    # Below are initialization points created from this input file
    # The variable IC should be set from_file_var for temperature and precursors
    # - vel_x, vel_y, p, T_fluid, c_i from cosine heated simulation
    # file = 'restart/run_ns_restart.e'
    # - vel_x, vel_y, p, T_fluid, c_i from coupled multiphysics simulation
    # file = 'restart/run_ns_coupled_restart.e'
  []
  [hx_top]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > 0'
    included_subdomains = '3'
    included_neighbors = '1'
    fixed_normal = true
    normal = '0 1 0'
    new_sideset_name = 'hx_top'
    input = 'restart'
  []
  [hx_bot]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y <-0.6'
    included_subdomains = '3'
    included_neighbors = '1'
    fixed_normal = true
    normal = '0 -1 0'
    new_sideset_name = 'hx_bot'
    input = 'hx_top'
  []
[]

################################################################################
# EQUATIONS: VARIABLES, KERNELS, BOUNDARY CONDITIONS
################################################################################

[Modules]
  [NavierStokesFV]
    # General parameters
    compressibility = 'incompressible'
    #compressibility = 'weakly-compressible'
    add_energy_equation = true
    add_scalar_equation = true
    boussinesq_approximation = true
    block = 'fuel pump hx'

    # Variables, defined below for the Exodus restart
    velocity_variable = 'vel_x vel_y'
    pressure_variable = 'pressure'
    fluid_temperature_variable = 'T_fluid'

    # Material properties
    density = ${rho}
    #density = DensFunc
    #dynamic_viscosity = ${mu}
    dynamic_viscosity = MU_aux
    #thermal_conductivity = ${k}
    thermal_conductivity = K_aux
    #specific_heat = 'cp'
    #specific_heat = Cp_aux
    specific_heat = ${cp}
    thermal_expansion = ${alpha}

    # Boussinesq parameters
    gravity = '0 -9.81 0'
    ref_temperature = ${T_HX}

    # Boundary conditions
    wall_boundaries = 'shield_wall reflector_wall fluid_symmetry'
    momentum_wall_types = 'wallfunction wallfunction symmetry'
    energy_wall_types = 'heatflux heatflux heatflux'
    energy_wall_function = '0 0 0'

    # Pressure pin for incompressible flow
    pin_pressure = true
    pinned_pressure_type = average
    pinned_pressure_value = 1e5

    # Turbulence parameters
    turbulence_handling = 'mixing-length'
    turbulent_prandtl = ${Pr_t}
    von_karman_const = ${von_karman_const}
    mixing_length_delta = 0.1
    mixing_length_walls = 'shield_wall reflector_wall'
    mixing_length_aux_execute_on = 'initial'

    # Numerical scheme
    momentum_advection_interpolation = 'upwind'
    mass_advection_interpolation = 'upwind'
    energy_advection_interpolation = 'upwind'
    passive_scalar_advection_interpolation = 'upwind'

    # Heat source
    external_heat_source = power_density

    # Precursor advection, diffusion and source term
    passive_scalar_names = 'c1 c2 c3 c4 c5 c6'
    passive_scalar_schmidt_number = '${Sc_t} ${Sc_t} ${Sc_t} ${Sc_t} ${Sc_t} ${Sc_t}'
    passive_scalar_coupled_source = 'fission_source c1; fission_source c2; fission_source c3;
                                     fission_source c4; fission_source c5; fission_source c6;'
    passive_scalar_coupled_source_coeff = '${beta1} ${lambda1_m}; ${beta2} ${lambda2_m};
                                           ${beta3} ${lambda3_m}; ${beta4} ${lambda4_m};
                                           ${beta5} ${lambda5_m}; ${beta6} ${lambda6_m}'

    # Heat exchanger
    friction_blocks = 'hx'
    friction_types = 'FORCHHEIMER'
    friction_coeffs = ${friction}
    ambient_convection_blocks = 'hx'
    ambient_convection_alpha = '${fparse 600 * 20e3}' # HX specifications
    ambient_temperature = ${T_HX}
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    block = 'fuel pump hx'
    initial_from_file_var = vel_x
  []
  [vel_y]
    type = INSFVVelocityVariable
    block = 'fuel pump hx'
    initial_from_file_var = vel_y
  []
  [pressure]
    type = INSFVPressureVariable
    block = 'fuel pump hx'
    initial_from_file_var = pressure
  []
  [T_fluid]
    type = INSFVEnergyVariable
    block = 'fuel pump hx'
    #initial_condition = ${T_HX}
    initial_from_file_var = T_fluid
  []

  [c1]
    type = MooseVariableFVReal
    block = 'fuel pump hx'
    # initial_from_file_var = c1
    [InitialCondition]
      type = FunctionIC
      function = 'cosine_guess'
      scaling_factor = 0.02
    []
  []
  [c2]
    type = MooseVariableFVReal
    block = 'fuel pump hx'
    # initial_from_file_var = c2
    [InitialCondition]
      type = FunctionIC
      function = 'cosine_guess'
      scaling_factor = 0.1
    []
  []
  [c3]
    type = MooseVariableFVReal
    block = 'fuel pump hx'
    # initial_from_file_var = c3
    [InitialCondition]
      type = FunctionIC
      function = 'cosine_guess'
      scaling_factor = 0.03
    []
  []
  [c4]
    type = MooseVariableFVReal
    block = 'fuel pump hx'
    # initial_from_file_var = c4
    [InitialCondition]
      type = FunctionIC
      function = 'cosine_guess'
      scaling_factor = 0.04
    []
  []
  [c5]
    type = MooseVariableFVReal
    block = 'fuel pump hx'
    # initial_from_file_var = c5
    [InitialCondition]
      type = FunctionIC
      function = 'cosine_guess'
      scaling_factor = 0.01
    []
  []
  [c6]
    type = MooseVariableFVReal
    block = 'fuel pump hx'
    # initial_from_file_var = c6
    [InitialCondition]
      type = FunctionIC
      function = 'cosine_guess'
      scaling_factor = 0.001
    []
  []
  [T_ref]
    type = INSFVEnergyVariable
    block = 'reflector shield'
    #scaling = 1e-6
    #initial_from_file_var=T_ref
    initial_condition = 500
  []
[]

[AuxVariables]
  [power_density]
    type = MooseVariableFVReal
    block = 'fuel pump hx'
    # Power density is re-initalized by a transfer from neutronics
    [InitialCondition]
      type = FunctionIC
      function = 'cosine_guess'
      scaling_factor = '${fparse 3e9/2.81543}'
    []
  []
  [fission_source]
    type = MooseVariableFVReal
    # Fission source is re-initalized by a transfer from neutronics
    [InitialCondition]
      type = FunctionIC
      function = 'cosine_guess'
      scaling_factor = '${fparse 6.303329e+01/2.81543}'
    []
    block = 'fuel pump hx'
  []
  ##############aggiungo io
  [MU_aux]
    type = INSFVScalarFieldVariable
    initial_condition = 0.003
    block = 'fuel pump hx'
  []

  [rho_aux]
    type = INSFVScalarFieldVariable
    initial_condition = 3000
    block = 'fuel pump hx'
  []

  [Cp_aux]
    type = INSFVScalarFieldVariable
    initial_condition = 640
    block = 'fuel pump hx'
  []

  [K_aux]
    type = INSFVScalarFieldVariable
    initial_condition = 0.4
    block = 'fuel pump hx'
  []

  [U_mag]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    block = 'fuel pump hx'
  []
[]

[AuxKernels]
  [compute_MU]
    type = ParsedAux
    variable = MU_aux
    coupled_variables = 'T_fluid'
    #constant_names = 'T'
    #constant_expressions = 'T_fluid'
    #variable='T_fluid'
    expression = '0.0001505*exp(26660/(8.314*T_fluid))'
    block = 'fuel pump hx'
  []

  [compute_rho]
    type = ParsedAux
    variable = rho_aux
    coupled_variables = 'T_fluid'
    expression = '4212.6-1.0686*T_fluid'
    block = 'fuel pump hx'
  []

  [compute_Cp]
    type = ParsedAux
    variable = Cp_aux
    coupled_variables = 'T_fluid'
    expression = '8900.439-13.779*T_fluid+0.00640*T_fluid*T_fluid-844375800/(T_fluid*T_fluid)'
    block = 'fuel pump hx'
  []

  [compute_K]
    type = ParsedAux
    variable = K_aux
    coupled_variables = 'T_fluid'
    expression = '5.6820-0.0087832*T_fluid+0.0000040967*T_fluid*T_fluid-576420/(T_fluid*T_fluid)'
    block = 'fuel pump hx'
  []
  [compute_Umag]
    type = VectorMagnitudeAux
    variable = U_mag
    x = vel_x
    y = vel_y
    block = 'fuel pump hx'
  []
[]
###########################

[Functions]
  # Guess to have a 3D power distribution
  [cosine_guess]
    type = ParsedFunction
    expression = 'max(0, cos(x*pi/2/1.2))*max(0, cos(y*pi/2/1.1))'
  []
  [DensFunc]
    type = ParsedFunction
    #expression = '4212.6-1.0686*x'
    expression = 5
    #symbol_names = 'T'
    #symbol_values = averageT
  []

  [CPFunc]
    type = ParsedFunction
    #expression = '8900.439-13.779*x+0.00640*x*x-844375800/(x*x)'
    expression = 5
    #symbol_names = 'T'
    #symbol_values = averageT

  []

  [KFunc]
    type = ParsedFunction
    #expression = '5.6820-0.0087832*x+0.0000040967*x*x-576420/(x*x)'
    expression = 5
    #symbol_names = 'T'
    #symbol_values = averageT
  []

  [muFunc]
    type = ParsedFunction
    #expression = '0.0001505*exp(26660/(8.314*x))'
    expression = 5
    #symbol_names = 'T'
    #symbol_values = averageT
  []
[]

[FVKernels]
  [pump]
    type = INSFVBodyForce
    variable = vel_y
    functor = ${pump_force}
    block = 'pump'
    momentum_component = 'y'
  []

  # Solid specific
  [heat_time_ref]
    type = INSFVEnergyTimeDerivative
    variable = T_ref
    cp = 880.0 # 1000
    rho = 3580
    #is_solid = true
    block = 'shield reflector'
  []
  [heat_diffusion_ref]
    type = FVDiffusion
    variable = T_ref
    coeff = 30.0
    block = 'shield reflector'
  []
[]

##########################

################################################################################
# MATERIALS
################################################################################

[FluidProperties]
  [fp]
    type = TemperaturePressureFunctionFluidProperties
    #cv = ${cp}
    cv = 600
    k = KFunc
    rho = DensFunc
    mu = muFunc
  []
[]

[Materials]

  [to_vars]
    type = FluidPropertiesMaterialPT
    fp = fp
    outputs = 'all'
    output_properties = 'density k cp cv viscosity e h'
    pressure = pressure
    temperature = T_fluid
    block = 'fuel pump hx'

    compute_entropy = false
    compute_sound_speed = false
  []

  # Most of these constants could be specified directly to the action
  [mu]
    type = ADGenericFunctorMaterial
    prop_names = 'mu'
    #prop_values = '${mu}'
    prop_values = MU_aux
    block = 'fuel pump hx'
  []

  [cp_solid]
    type = ADGenericFunctorMaterial
    prop_names = 'cp_solid'
    prop_values = 880
    block = 'shield reflector'
  []
  [rho_solid]
    type = ADGenericFunctorMaterial
    prop_names = 'rho_solid'
    prop_values = 5000
    block = 'shield reflector'
  []
  [k_solid]
    type = ADGenericFunctorMaterial
    prop_names = 'k_solid'
    prop_values = 30
    block = 'shield reflector'
  []
  [cp]
    type = ADGenericFunctorMaterial
    prop_names = 'cp'
    #prop_values = '${cp}'
    prop_values = Cp_aux
    block = 'fuel pump hx'
  []
[]

################################################################################
# EXECUTION / SOLVE
################################################################################

[Functions]
  [dts]
    type = PiecewiseConstant
    x = '0    100'
    y = '0.75 2.5'
  []
[]

[Executioner]
  type = Transient

  # Time stepping parameters
  start_time = 0.0
  end_time = 2
  # end_time will depend on the restart file chosen
  # though steady state detection can also be used
  # from _initial/no heating : 150 - 200s enough
  # from _ns/_ns_coupled/heated: 10s enough

  [TimeStepper]
    # This time stepper makes the time step grow exponentially
    # It can only be used with proper initialization
    type = IterationAdaptiveDT
    dt = 1 # chosen to obtain convergence with first coupled iteration
    growth_factor = 2
  []
  # [TimeStepper]
  #   type = FunctionDT
  #   function = dts
  # []
  steady_state_detection = true
  #steady_state_tolerance  = 1e-8
  steady_state_tolerance = 1e-4
  steady_state_start_time = 10

  # Time integration scheme
  scheme = 'implicit-euler'

  # Solver parameters
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'lu NONZERO 50'
  line_search = 'none'

  #nl_rel_tol = 1e-9
  #nl_abs_tol = 2e-8
  nl_rel_tol = 1e-5
  nl_abs_tol = 2e-4
  #nl_max_its = 15
  nl_max_its = 20
  l_max_its = 50

  automatic_scaling = true
  # resid_vs_jac_scaling_param = 1
[]

################################################################################
# SIMULATION OUTPUTS
################################################################################

[Outputs]
  csv = true
  hide = 'flow_hx_bot flow_hx_top min_flow_T max_flow_T'
  #execute_on = 'final'
  [restart]
    type = Exodus
    overwrite = true
  []
  # Reduce base output
  print_linear_converged_reason = false
  print_linear_residuals = false
  print_nonlinear_converged_reason = false
[]

[Postprocessors]
  [max_v]
    type = ElementExtremeValue
    variable = vel_x
    value_type = max
    block = 'fuel pump hx'
  []
  # TODO: weakly compressible, switch to mass flow rate
  [flow_hx_bot]
    type = VolumetricFlowRate
    boundary = 'hx_bot'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 1
  []
  [flow_hx_top]
    type = VolumetricFlowRate
    boundary = 'hx_top'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 1
  []
  [max_flow_T]
    type = VolumetricFlowRate
    boundary = 'hx_top'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 'T_fluid'
  []
  [min_flow_T]
    type = VolumetricFlowRate
    boundary = 'hx_bot'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 'T_fluid'
  []
  [dT]
    type = ParsedPostprocessor
    function = '-max_flow_T / flow_hx_bot + min_flow_T / flow_hx_top'
    pp_names = 'max_flow_T min_flow_T flow_hx_bot flow_hx_top'
  []
  [total_power]
    type = ElementIntegralVariablePostprocessor
    variable = power_density
    block = 'fuel pump hx'
  []
  [total_fission_source]
    type = ElementIntegralVariablePostprocessor
    variable = fission_source
    block = 'fuel pump hx'
  []
  [averageT]
    type = ElementAverageValue
    variable = T_fluid
    block = 'fuel pump hx'
    execute_on = 'INITIAL'
  []

  [integral_pp]
    type = FunctionElementIntegral
    function = CPFunc
    #execute_on = 'INITIAL'
    block = 'fuel pump hx'
  []

  [AV_Cp]
    type = FunctionValuePostprocessor
    function = CPFunc
  []

  [AV_K]
    type = FunctionValuePostprocessor
    function = KFunc
  []

  [AV_mu]
    type = FunctionValuePostprocessor
    function = muFunc
  []

  [AV_dena]
    type = FunctionValuePostprocessor
    function = DensFunc
  []

  [muProc]
    type = PointValue
    point = '0.225  0.42  0'
    variable = MU_aux
  []
  [KProc]
    type = PointValue
    point = '0.225  0.42  0'
    variable = K_aux
  []
  [CpProc]
    type = PointValue
    point = '0.225  0.42  0'
    variable = Cp_aux
  []

  [U_magnitude]
    type = PointValue
    point = '0.225  0.42  0'
    variable = U_mag
  []

[]

[FVInterfaceKernels]
  [convection]
    type = FVConvectionCorrelationInterface
    variable1 = T_fluid
    variable2 = T_ref
    boundary = 'reflector_wall'
    h = 300
    T_solid = T_ref
    T_fluid = T_fluid
    subdomain1 = 'fuel'
    subdomain2 = 'reflector'
    wall_cell_is_bulk = true
  []
[]

[FVBCs]
  #[internal_temperature]
  #  type = MatchedValueBC
  #  variable = T_ref
  #  v = T_fluid
  #  boundary = 'reflector_wall'
  #[]
  [external_cooling]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = 'outer'
    variable = T_ref
    T_bulk = 300
    T_solid = T_ref
    is_solid = true
    heat_transfer_coefficient = 30
  []
  # [external_cooling]
  # type = ConvectiveHeatFluxBC
  # variable = T_ref
  # heat_transfer_coefficient = 30.0     #prima era 1e-3
  # heat_transfer_coefficient_dT = 0
  # T_infinity = 300.0
  # boundary = 'outer'
  #[]
[]
