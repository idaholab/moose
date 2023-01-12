# This model drives a multiapp setup where wrappers (solid heat conduction)
# and interwrapper flow (fluid) are solved in this input. Intra-element flow is solved
# by subchannel
# heat is applied in the pin model in subchannel

# Units are SI

# physics parameters
inlet_temperature = 628.15
outlet_pressure = 758423 # Pa # gauge pressure

# geometry parameters
pin_diameter = 8e-3
pin_pitch = 9.04e-3
# cr_pin_diameter = 1.11e-2
# cr_pin_pitch = 1.2476e-2
flat_to_flat = 13.598e-2
wire_diameter = 1.03e-3
wire_pitch = 203.2e-3
inter_assembly_gap = 4e-3
length = 2.6

# fluid properties
rho = 820
mu = 2.27e-4
cp = 1260
k = 65
molar_mass = 22.989769e-3

# wrapper properties
k_wrapper = 20
cp_wrapper = 300
rho_wrapper = 7800

# hydraulic diameters
D_hydraulic_interwrapper = ${fparse 2 * inter_assembly_gap}
D_hydraulic_fuel = 0.00297

wrapper_blocks = 'wrapper'
flow_blocks = 'interwrapper porous_flow center_porous_flow'

[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = 'seven_wrapper_interwrapper2_in.e'
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[GlobalParams]
  mu_rampdown = mu_rampdown_fn
  rhie_chow_user_object = pins_rhie_chow_interpolator
[]

[Debug]
  show_material_props = true
[]

[FluidProperties]
  [fp]
    type = SimpleFluidProperties
    cp = ${cp}
    cv = ${cp}
    thermal_conductivity = ${k}
    density0 = ${rho}
    viscosity = ${mu}
    molar_mass = ${molar_mass}
    thermal_expansion = 0
    bulk_modulus = 1e16
  []
[]

[UserObjects]
  [fuel_hex]
    type = HexagonalLattice
    flow_region_flat_to_flat = ${flat_to_flat}
    pin_pitch = ${pin_pitch}
    pin_diameter = ${pin_diameter}
    wire_diameter = ${wire_diameter}
    wire_pitch = ${wire_pitch}
    n_rings = 9
  []
[]

[Functions]
  [inlet_vel_interwrapper_fn]
    type = PiecewiseLinear
    x = '1'
    y = '0.1'
  []

  [inlet_vel_assembly_fn]
    type = PiecewiseLinear
    x = '1'
    y = '0.5'
  []

  # [dt_fn]
  #   type = PiecewiseLinear
  #   x = '0    1      5 50 100'
  #   y = '0.2  0.2  0.5 1  10'
  # []

  [mu_rampdown_fn]
    type = PiecewiseLinear
    x = '10'
    y = '1'
  []
[]

[Variables]
  [T_wrapper]
    type = MooseVariableFVReal
    initial_condition = ${inlet_temperature}
    block = ${wrapper_blocks}
  []
[]

[FVBCs]
  active =''
  [T_wrapper_inside_wall]
    type = FVFunctorDirichletBC
    variable = T_wrapper
    functor = duct_surface_temperature_functor
    boundary = 'inner_wall'
  []
[]

[FVKernels]
  inactive ='solid_energy_time'
  [solid_energy_time]
    type = PINSFVEnergyTimeDerivative
    variable = T_wrapper
    cp = ${cp_wrapper}
    rho = ${rho_wrapper}
    is_solid = true
    porosity = 0
    block = ${wrapper_blocks}
  []

  [solid_heat_conduction]
    type = FVDiffusion
    variable = T_wrapper
    coeff = ${k_wrapper}
    block = ${wrapper_blocks}
  []
[]

[FVInterfaceKernels]
  active = ''
  [conjugate_ht]
    type = FVConvectionCorrelationInterface
    wall_cell_is_bulk = true
    variable1 = T_fluid
    subdomain1 = ${flow_blocks}
    variable2 = T_wrapper
    subdomain2 = ${wrapper_blocks}
    T_fluid = T_fluid
    T_solid = T_wrapper
    h = wall_htc
    boundary = 'conjugate_heat_transfer_boundary'
  []
[]

[Modules]
  [NavierStokesFV]
    # general input parameters
    compressibility = 'incompressible'
    add_energy_equation = true
    block = ${flow_blocks}

    # porous media parameters
    porous_medium_treatment = true
    porosity = porosity
    friction_types = 'darcy forchheimer'
    friction_coeffs = 'Darcy_coefficient Forchheimer_coefficient'

    # discretization parameters
    energy_advection_interpolation = upwind
    pressure_face_interpolation = average
    momentum_advection_interpolation = upwind
    mass_advection_interpolation = upwind

    # material properties
    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k k k'
    # defined on all flow blocks
    thermal_conductivity_blocks = 'interwrapper; porous_flow; center_porous_flow'
    specific_heat = 'cp'

    # initial conditions
    initial_velocity = '0 0 1e-4'
    initial_temperature = '${inlet_temperature}'
    initial_pressure = '${outlet_pressure}'

    # boundary conditions
    inlet_boundaries =     'inlet_interwrapper               inlet_assembly '
    momentum_inlet_types = 'fixed-velocity                   fixed-velocity'
    momentum_inlet_function = '0 0 inlet_vel_interwrapper_fn; 0 0 inlet_vel_assembly_fn'

    energy_inlet_types = 'fixed-temperature fixed-temperature'
    energy_inlet_function = '${inlet_temperature} ${inlet_temperature}'

    wall_boundaries = 'outside_circumference conjugate_heat_transfer_boundary'
    momentum_wall_types = 'slip slip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'outlet'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${outlet_pressure}'
  []
[]

[Materials]
  ## the fluid properties
  ## need to block restrict to accomodate different hydraulic diameters
  [fluid_props_fuel]
    type = GeneralFunctorFluidProps
    T_fluid = T_fluid
    pressure = pressure
    porosity = porosity
    speed = speed
    fp = fp
    characteristic_length = ${D_hydraulic_fuel}
    block = 'porous_flow center_porous_flow'
  []

  [fluid_props_interwrapper]
    type = GeneralFunctorFluidProps
    T_fluid = T_fluid
    pressure = pressure
    porosity = porosity
    speed = speed
    fp = fp
    characteristic_length = ${D_hydraulic_interwrapper}
    block = 'interwrapper'
  []

  ## set the blockwise porosity
  [porosity_functor_mat]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'porosity'
    subdomain_to_prop_value = 'interwrapper 1
                               porous_flow  0.3
                               center_porous_flow 0.3'
    block = ${flow_blocks}
  []

  ## wall heat transfer coefficient uses Lyon-Martinelli
  [wall_htc]
    type = FunctorLyonMartinelliWallHTC
    block = ${flow_blocks}
  []

  ## set characteristic length on each block
  [characteristic_length]
    type = PiecewiseByBlockFunctorMaterial
    prop_name =  'characteristic_length'
    subdomain_to_prop_value = 'interwrapper ${D_hydraulic_interwrapper}
                               porous_flow  ${D_hydraulic_fuel}
                               center_porous_flow ${D_hydraulic_fuel}'
    block = ${flow_blocks}
  []

  # Flow resistances
  [interwrapper_drag]
    type = FunctorChurchillDragCoefficients
    block = 'interwrapper'
  []

  [assembly_drag]
    type = FunctorRehmeDragCoefficients
    multipliers = '100 100 1'
    hex_lattice = fuel_hex
    block = 'porous_flow'
  []

  [assembly_drag2]
    type = FunctorRehmeDragCoefficientsPressureGradient
    multipliers = '100 100 1'
    hex_lattice = fuel_hex
    postprocessor = report_pressure_drop
    L = ${length}
    block = 'center_porous_flow'
  []

  # get duct_surface_temperature_functor
  # need to play this stupid game here to have it accepted
  # by the BC
  [converter_to_regular]
    type = FunctorADConverter
    ad_props_in = 'duct_surface_temperature'
    reg_props_out = 'duct_surface_temperature_functor'
  []
[]

[AuxVariables]
  [duct_surface_temperature]
    type = MooseVariableFVReal
  []

  [q_prime_duct]
    type = MooseVariableFVReal
    initial_condition = 0
  []

  # needed because T_wrapper gradient is zero
  # in FEM loop
  # how can we directly use T_wrapper to compute the
  # qdot'
  [T_wrapper_linear]
    block = ${wrapper_blocks}
    initial_condition = ${inlet_temperature}
  []

  [rho_cp_T_fluid_var]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [rho_var]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [cp_var]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []
[]

[AuxKernels]
  [QPrime]
    type = QPrimeDuctAux
    diffusivity = ${k_wrapper}
    flat_to_flat = ${flat_to_flat}
    variable = q_prime_duct
    # FIXME: need to do this because T_wrapper has zero
    #        gradient
    diffusion_variable = T_wrapper_linear
    component = normal
    boundary = 'inner_wall'
    execute_on = 'timestep_end'
  []

  [rho_var_aux]
    type = ADFunctorElementalAux
    variable = rho_var
    functor = rho
    block = ${flow_blocks}
  []

  [cp_var_aux]
    type = ADFunctorElementalAux
    variable = cp_var
    functor = cp
    block = ${flow_blocks}
  []

  [rho_cp_T_fluid_var_aux]
    type = ParsedAux
    variable = rho_cp_T_fluid_var
    args = 'rho_var cp_var T_fluid'
    function = 'rho_var * cp_var * T_fluid'
    block = ${flow_blocks}
  []
[]

[Postprocessors]
  [outlet_average_pressure_Pr]
    type = SideAverageValue
    boundary = outlet_central_assembly
    variable = pressure
  []

  [inlet_average_pressure_Pr]
    type = SideAverageValue
    boundary = inlet_central_assembly
    variable = pressure
  []

  [inlet_central_mfr]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho
    boundary = 'inlet_central_assembly'
  []

  [central_assembly_area]
    type = AreaPostprocessor
    boundary = inlet_central_assembly
  []

  [inlet_mass_flux_Pr]
    type = ParsedPostprocessor
    pp_names = 'inlet_central_mfr central_assembly_area'
    function = 'abs(inlet_central_mfr/central_assembly_area)'
  []

  [pressure_drop_Pr]
    type = ParsedPostprocessor
    pp_names = 'inlet_average_pressure_Pr outlet_average_pressure_Pr'
    function = 'inlet_average_pressure_Pr - outlet_average_pressure_Pr'
  []

  [report_pressure_drop]
    type = Receiver
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       100                 NONZERO               1e-10   '
  end_time = 1.0
  l_max_its = 10
  [TimeStepper]
   type = IterationAdaptiveDT
    dt = 0.01
    iteration_window = 2
    optimal_iterations = 6
    growth_factor = 1.2
    cutback_factor = 0.8
  []
  nl_abs_tol = 1e-3
  nl_max_its = 15
[]

[Outputs]
  exodus = true
  #print_linear_residuals = false
[]

################################################################################
# A multiapp that couples Pronghorn to subchannel
################################################################################

[MultiApps]
  [subchannel]
    # app_type = SubchannelApp
    type = FullSolveMultiApp
    input_files = 'subchannel.i'
    execute_on = 'timestep_end'
    positions = '0 0 0'
    output_in_position = true
    bounding_box_padding = '0 0 0.1'
  []
[]

[Transfers]
  [pressure_drop_transfer] # Get pressure drop to pronghorn from subchannel
    type = MultiAppPostprocessorTransfer
    from_multi_app =  subchannel
    from_postprocessor = total_pressure_drop_SC
    to_postprocessor = report_pressure_drop
    reduction_type = average
    execute_on = 'timestep_end'
  []

  [mass_flux_tranfer] # Send mass_flux at the inlet to subchannel
    type = MultiAppPostprocessorTransfer
    to_multi_app = subchannel
    from_postprocessor = inlet_mass_flux_Pr
    to_postprocessor = report_mass_flux_inlet
    execute_on = 'timestep_end'
  []

  [outlet_pressure_tranfer] # Send pressure at the outlet to subchannel
    type = MultiAppPostprocessorTransfer
    to_multi_app = subchannel
    from_postprocessor = outlet_average_pressure_Pr
    to_postprocessor = report_pressure_outlet
    execute_on = 'timestep_end'
  []
[]
