# This model drives a multiapp setup where wrappers (solid heat conduction)
# and interwrapper flow (fluid) are solved in this input. Intra-element flow is solved
# by subchannel.

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
flow_blocks = 'interwrapper center_porous_flow'

[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = 'one_wrapper_interwrapper_in.e'
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
    thermal_conductivity = 'k k'
    # defined on all flow blocks
    thermal_conductivity_blocks = 'interwrapper; center_porous_flow'
    specific_heat = 'cp'

    # initial conditions
    initial_velocity = '0 0 1e-4'
    initial_temperature = '${inlet_temperature}'
    initial_pressure = '${outlet_pressure}'

    # boundary conditions
    inlet_boundaries =     'inlet_interwrapper               inlet_central_assembly '
    momentum_inlet_types = 'fixed-velocity                   fixed-velocity'
    momentum_inlet_function = '0 0 inlet_vel_interwrapper_fn; 0 0 inlet_vel_assembly_fn'

    energy_inlet_types = 'fixed-temperature fixed-temperature'
    energy_inlet_function = '${inlet_temperature} ${inlet_temperature}'

    wall_boundaries = 'inner_wall outer_wall'
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
    block = 'center_porous_flow'
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
                               center_porous_flow ${D_hydraulic_fuel}'
    block = ${flow_blocks}
  []

  # Flow resistances
  [interwrapper_drag]
    type = FunctorChurchillDragCoefficients
    block = 'interwrapper'
  []

  [center_assembly_drag]
    type = FunctorRehmeDragCoefficientsPressureGradient
    multipliers = '100 100 1'
    hex_lattice = fuel_hex
    pressure_drop_postprocessor = report_pressure_drop
    L = ${length}
    block = 'center_porous_flow'
  []

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
    coupled_variables = 'rho_var cp_var T_fluid'
    expression = 'rho_var * cp_var * T_fluid'
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

  [pressure_drop_Pr]
    type = ParsedPostprocessor
    pp_names = 'inlet_average_pressure_Pr outlet_average_pressure_Pr'
    function = 'inlet_average_pressure_Pr - outlet_average_pressure_Pr'
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

  [report_pressure_drop]
    type = Receiver
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       100                 NONZERO               1e-10   '
  end_time = 0.04
  l_max_its = 10
  [TimeStepper]
   type = IterationAdaptiveDT
    dt = 0.01
    iteration_window = 2
    optimal_iterations = 6
    growth_factor = 1.0
    cutback_factor = 1.0
  []
  nl_abs_tol = 1e-4
  nl_max_its = 15
[]

[Outputs]
  exodus = true
[]

################################################################################
# A multiapp that couples Pronghorn to subchannel
################################################################################

[MultiApps]
  [subchannel]
    type = FullSolveMultiApp
    input_files = 'subchannel.i'
    execute_on = 'timestep_end'
    positions = '0 0 0'
    max_procs_per_app = 1
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
