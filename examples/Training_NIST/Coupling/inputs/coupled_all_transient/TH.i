################################################################################
# Incompressible turbulent fluid flow for standalone simulations
################################################################################

################################################################################
# WARNING: You need to increase the derivative container size for this
# Recommended value: 200
# Use ./configure --with-derivative-size=200 command in the moose root directory
################################################################################

################################################################################
# Parameters for boundary/initial conditions and material properties
################################################################################
u_inlet = 5.0
p_outlet = 1.2e6
T_inlet = 350
visocity = 0.01

rho_UMo = 17045.8
cp_UMo = 141.29
k_UMo = 16.6821

rho_Al = 2702
cp_Al = 943.4
k_Al = 183.92

h_clad_water = 3000

[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = '../coupled_all_steady/TH_restart.e'
    use_for_exodus_restart = true
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'weakly-compressible'
    add_energy_equation = true
    block = 'water'
    initialize_variables_from_mesh_file = true

    density = 'rho' # Created by the fluid property
    dynamic_viscosity = ${visocity}
    thermal_conductivity = 'k' # Created by the fluid property
    specific_heat = 'cp' # Created by the fluid property

    turbulence_handling = mixing-length
    mixing_length_delta = 0.005
    mixing_length_walls = 'clad_wall assembly_wall'

    inlet_boundaries = 'fluid-inlet'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '0 0 speed_reduction'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = ${T_inlet}

    wall_boundaries = 'clad_wall assembly_wall left right'
    momentum_wall_types = 'noslip noslip symmetry symmetry '
    energy_wall_types = 'heatflux heatflux heatflux heatflux'
    energy_wall_function = '0 0 0 0'

    outlet_boundaries = 'fluid-outlet'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${p_outlet}'

    energy_advection_interpolation = 'upwind'
    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'

    gravity = '0 0 -9.81'

    energy_scaling = 1e-5
    mass_scaling = 1e-2
  []
[]

[Functions]
  [speed_reduction]
    type = PiecewiseLinear
    x = '0 2 5 10'
    y = '${u_inlet} ${u_inlet} ${fparse u_inlet/2} ${fparse u_inlet/2}'
  []
[]

[Variables]
  [T_solid]
    type = INSFVEnergyVariable
    block = 'clad fuel'
    scaling = '1e-3'
    initial_from_file_var = T_solid
  []
[]

[AuxVariables]
  [power_density]
    type = MooseVariableFVReal
    block = 'fuel'
    initial_from_file_var = power_density
  []
  [Tfuel]
    block = fuel
    order = CONSTANT
    family = MONOMIAL
  []
  [Tclad]
    block = clad
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [Tfuel_aux]
    type = CopyAux
    variable = Tfuel
    copy_variable = T_solid
  []
  [Tclad_aux]
    type = CopyAux
    variable = Tclad
    copy_variable = T_solid
  []
[]

[FVKernels]
  [time_derivative]
    type = PINSFVEnergyTimeDerivative
    variable = T_solid
    cp = 'cp_solid'
    rho = 'rho_solid'
    is_solid = true
    porosity = 0.0
    block = 'fuel clad'
  []
  [solid_energy_diffusion]
    type = FVDiffusion
    coeff = k_solid
    variable = T_solid
    block = 'fuel clad'
  []
  [solid_heat_source]
    type = FVCoupledForce
    variable = T_solid
    v = power_density
    block = 'fuel'
  []
[]

[FVInterfaceKernels]
  [convection]
    type = FVConvectionCorrelationInterface
    variable1 = T_fluid
    variable2 = T_solid
    subdomain1 = 'water'
    subdomain2 = 'clad'
    boundary = clad_wall
    h = ${h_clad_water}
    T_solid = T_solid
    T_fluid = T_fluid
    wall_cell_is_bulk = true
  []
[]

[FluidProperties]
  [water_properties]
    type = Water97FluidProperties
  []
[]

[Materials]
  [k_solid]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'k_solid'
    subdomain_to_prop_value = 'fuel ${k_UMo} clad ${k_Al}'
    block = 'fuel clad'
  []
  [rho_solid]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'rho_solid'
    subdomain_to_prop_value = 'fuel ${rho_UMo} clad ${rho_Al}'
    block = 'fuel clad'
  []
  [cp_solid]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'cp_solid'
    subdomain_to_prop_value = 'fuel ${cp_UMo} clad ${cp_Al}'
    block = 'fuel clad'
  []
  [fluid_props]
    type = GeneralFunctorFluidProps
    fp = water_properties
    characteristic_length = 1
    porosity = 1
    pressure = 'pressure'
    speed = 'speed'
    T_fluid = 'T_fluid'
    block = 'water'
  []
  [speed]
    type = PINSFVSpeedFunctorMaterial
    porosity = 1
    superficial_vel_x = vel_x
    superficial_vel_y = vel_y
    superficial_vel_z = vel_z
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu  NONZERO superlu_dist'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  line_search = none
  end_time = 10.0

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.25
    growth_factor = 10.0
    cutback_factor = 0.5
    optimal_iterations = 6
    iteration_window = 1
  []
[]

[MultiApps]
  [thermo_mechanics]
    type = TransientMultiApp
    input_files = 'TM.i'
    execute_on = 'TIMESTEP_END'
  []
[]

[Transfers]
  [T_solid_transfer]
    type = MultiAppGeometricInterpolationTransfer
    to_multi_app = thermo_mechanics
    variable = temperature
    source_variable = T_solid
  []
  [p_transfer]
    type = MultiAppGeometricInterpolationTransfer
    to_multi_app = thermo_mechanics
    variable = pressure
    source_variable = pressure
  []
[]

[Postprocessors]
  [average_fuel_T]
    type = ElementAverageValue
    block = 'fuel'
    variable = T_solid
    execute_on = 'initial timestep_end'
  []
  [max_fuel_T]
    type = ElementExtremeValue
    block = 'fuel'
    value_type = max
    variable = T_solid
    execute_on = 'initial timestep_end'
  []
  [average_clad_T]
    type = ElementAverageValue
    block = 'clad'
    variable = T_solid
    execute_on = 'initial timestep_end'
  []
  [max_clad_T]
    type = ElementExtremeValue
    block = 'clad'
    value_type = max
    variable = T_solid
    execute_on = 'initial timestep_end'
  []
  [peacking_factor_fuel]
    type = ParsedPostprocessor
    pp_names = 'max_fuel_T average_fuel_T'
    function = max_fuel_T/average_fuel_T
    execute_on = 'initial timestep_end'
  []
  [peacking_factor_clad]
    type = ParsedPostprocessor
    pp_names = 'max_clad_T average_clad_T'
    function = max_clad_T/average_clad_T
    execute_on = 'initial timestep_end'
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  csv = true
[]