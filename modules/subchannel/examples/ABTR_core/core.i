# Flow in a minicore with heat, but no
# interwrapper flow, single porosity region per assembly

# Units are SI

# geometry parameters
pin_diameter = 8e-3
pin_pitch = 9.04e-3
cr_pin_diameter = 1.11e-2
cr_pin_pitch = 1.2476e-2
flat_to_flat = 13.598e-2
wire_diameter = 1.03e-3
wire_pitch = 203.2e-3

# physics parameters
inlet_temperature = 628.15
outlet_pressure = 0 # gauge pressure

# fluid properties
rho = 820
mu = 2.27e-4
cp = 1260
k = 65
molar_mass = 22.989769e-3

# wrapper/interwrapper/properties
k_wrapper = 40
cp_wrapper = 300
rho_wrapper = 2000

# fuel rod properties
k_fuel = 10
cp_fuel = 300
rho_fuel = 15000

# heat source
heat_source = 1e8

## block definitions

# the blocks above and below the ducted region
pool_flow_blocks = 'cold_pool1 cold_pool2 hot_pool1 hot_pool2 inlet_duct'

# all blocks which define some sort of flow
flow_blocks = 'fuel coupled_fuel control free_duct cold_pool1 cold_pool2 hot_pool1 hot_pool2
               cr_tip fuel_orifice control_orifice inlet_duct'

# Blocks where we have either fuel or control rods
# on these blocks we define T_solid & have conjugate heat transfer with the fluid
#                    and use Rehme drag coefficients
rodded_flow_blocks = 'fuel coupled_fuel control'

# Complement to the rodded_flow_blocks
non_rodded_flow_blocks = 'free_duct cold_pool1 cold_pool2 hot_pool1 hot_pool2
                          cr_tip fuel_orifice control_orifice inlet_duct'

# this is the only entirely solid block
wrapper_blocks = 'wrapper_interwrapper'

# hydraulic diameter of the assembly (check these again)
D_hydraulic_fuel = 0.00297
D_hydraulic_control = 0.00791
D_hydraulic_hotcoldpools = 0.34
D_hydraulic_empty_duct = ${flat_to_flat}

# flow control parameters to control assemblies
cntrl_orifice_gamma = 0.2
control_Forchheimer = 500

# mass flow rate: 33 fuel assemblies, 21 kg/s per element, and 1.2 to account for flow in
# CR assembly
mdot = ${fparse 33 * 21 * 1.2}
inlet_area = 0.564236
inlet_vel = ${fparse -mdot / inlet_area / rho}

[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = 'abtr_core_mesh_in.e'
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

  [cr_hex]
    type = HexagonalLattice
    flow_region_flat_to_flat = ${flat_to_flat}
    pin_pitch = ${cr_pin_pitch}
    pin_diameter = ${cr_pin_diameter}
    wire_diameter = 0
    wire_pitch = 1
    n_rings = 6
  []
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

[Functions]
  [inlet_vel_fn]
    type = PiecewiseLinear
    x = '0      1'
    y = '-1e-15 ${inlet_vel}'
  []

  [dt_fn]
    type = PiecewiseLinear
    x = '0    1      5 50 100'
    y = '0.2  0.2  0.5 1  10'
  []

  [mu_rampdown_fn]
    type = PiecewiseLinear
    x = '0    0.5  1   5  10'
    y = '1000 1000 100 10 1'
  []

  [heat_source_fn]
    type = PiecewiseLinear
    x = '0 1 10'
    y = '0 0 ${heat_source}'
  []
[]

[Variables]
  [T_wrapper]
    type = MooseVariableFVReal
    initial_condition = ${inlet_temperature}
    block = ${wrapper_blocks}
  []

  [T_solid]
    type = INSFVEnergyVariable
    initial_condition = ${inlet_temperature}
    block = ${rodded_flow_blocks}
  []
[]

[FVKernels]
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

  [solid_energy_time_fluid_block]
    type = PINSFVEnergyTimeDerivative
    variable = T_solid
    cp = ${cp_fuel}
    rho = ${rho_fuel}
    is_solid = true
    porosity = porosity
    block = ${rodded_flow_blocks}
  []

  [solid_heat_conduction_fluid_block]
    type = PINSFVEnergyAnisotropicDiffusion
    kappa = 'kappa_s'
    variable = T_solid
    porosity = porosity
    block = ${rodded_flow_blocks}
  []

  [solid_heat_source_fluid_block]
    type = FVBodyForce
    variable = T_solid
    function = heat_source_fn
    # heat is only applied in the fuel blocks
    block = 'fuel coupled_fuel'
  []

  [fluid_to_solid_convection]
    type = PINSFVEnergyAmbientConvection
    variable = T_solid
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = true
    h_solid_fluid = alpha
    block = ${rodded_flow_blocks}
  []
[]

[FVInterfaceKernels]
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
    boundary = 'interior_walls'
  []
[]

[AuxVariables]
  [porosity_var]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [Re_i_var]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [Forchheimer_z]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [Darcy_z]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [interstitial_vel_z]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [alpha_var]
    type = MooseVariableFVReal
    block = ${rodded_flow_blocks}
  []

  [wall_htc_var]
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

  [rho_cp_T_fluid_var]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [kappa_var_x]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [kappa_var_z]
    type = MooseVariableFVReal
    block = ${flow_blocks}
  []

  [q_prime_duct_out]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [porosity_var_aux]
    type = ADFunctorElementalAux
    variable = porosity_var
    functor = porosity
    block = ${flow_blocks}
  []

  [Re_i_var_aux]
    type = ADFunctorElementalAux
    variable = Re_i_var
    functor = Re_i
    block = ${flow_blocks}
  []

  [Forchheimer_z_var_aux]
    type = ADFunctorVectorElementalAux
    variable = Forchheimer_z
    functor = Forchheimer_coefficient
    component = 2
    block = ${flow_blocks}
  []

  [Darcy_z_var_aux]
    type = ADFunctorVectorElementalAux
    variable = Darcy_z
    functor = Darcy_coefficient
    component = 2
    block = ${flow_blocks}
  []

  [kappa_var_x_aux]
    type = ADFunctorVectorElementalAux
    variable = kappa_var_x
    functor = kappa
    component = 0
    block = ${flow_blocks}
  []

  [kappa_var_z_aux]
    type = ADFunctorVectorElementalAux
    variable = kappa_var_z
    functor = kappa
    component = 2
    block = ${flow_blocks}
  []

  [interstitial_vel_z_aux]
    type = ADFunctorVectorElementalAux
    variable = interstitial_vel_z
    functor = velocity
    component = 2
    block = ${flow_blocks}
  []

  [alpha_var_aux]
    type = ADFunctorElementalAux
    variable = alpha_var
    functor = alpha
    block = ${rodded_flow_blocks}
  []

  [wall_htc_var_aux]
    type = ADFunctorElementalAux
    variable = wall_htc_var
    functor = wall_htc
    block = ${flow_blocks}
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
    porosity_smoothing_layers = 10
    use_friction_correction = true
    consistent_scaling = 50
    # conjugate heat transfer T_sold w/ T_fluid
    ambient_convection_alpha = 'alpha alpha alpha'
    # defined on the porous flow blocks
    ambient_convection_blocks = 'fuel; coupled_fuel; control'
    ambient_temperature = 'T_solid T_solid T_solid'

    # discretization parameters
    energy_advection_interpolation = upwind
    pressure_face_interpolation = average
    momentum_advection_interpolation = upwind
    mass_advection_interpolation = upwind


    # material properties
    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'kappa kappa kappa kappa kappa kappa
                            kappa kappa kappa kappa kappa kappa'
    # defined on all flow blocks
    thermal_conductivity_blocks = 'fuel;
                                   coupled_fuel;
                                   control;
                                   free_duct;
                                   cold_pool1;
                                   cold_pool2;
                                   hot_pool1;
                                   hot_pool2;
                                   cr_tip;
                                   fuel_orifice;
                                   control_orifice;
                                   inlet_duct'
    specific_heat = 'cp'

    # initial conditions
    initial_velocity = '1e-15 1e-15 1e-15'
    initial_temperature = '${inlet_temperature}'
    initial_pressure = '${outlet_pressure}'

    # boundary conditions
    inlet_boundaries = 'inlet'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '0 0 inlet_vel_fn'

    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '${inlet_temperature}'

    wall_boundaries = 'interior_walls interior_walls_coupled hot_pool_wall cold_pool_wall'
    momentum_wall_types = 'slip slip slip slip'
    energy_wall_types = 'heatflux heatflux heatflux heatflux'
    energy_wall_function = '0 0 0 0'

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
    block = 'fuel coupled_fuel fuel_orifice'
  []

  [fluid_props_control]
    type = GeneralFunctorFluidProps
    T_fluid = T_fluid
    pressure = pressure
    porosity = porosity
    speed = speed
    fp = fp
    characteristic_length = ${D_hydraulic_control}
    block = 'control control_orifice'
  []

  [fluid_props_empty_hex_duct]
    type = GeneralFunctorFluidProps
    T_fluid = T_fluid
    pressure = pressure
    porosity = porosity
    speed = speed
    fp = fp
    characteristic_length = ${D_hydraulic_empty_duct}
    block = 'free_duct cr_tip'
  []

  [fluid_props_pools]
    type = GeneralFunctorFluidProps
    T_fluid = T_fluid
    pressure = pressure
    porosity = porosity
    speed = speed
    fp = fp
    characteristic_length = ${D_hydraulic_hotcoldpools}
    block = ${pool_flow_blocks}
  []

  ## set the blockwise porosity
  [porosity_functor_mat]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'porosity'
    subdomain_to_prop_value = 'fuel            0.31
                               coupled_fuel    0.31
                               fuel_orifice    1
                               control         0.45
                               control_orifice ${cntrl_orifice_gamma}
                               cold_pool1      1
                               cold_pool2      1
                               hot_pool1       1
                               hot_pool2       1
                               inlet_duct      1
                               free_duct       1
                               cr_tip          1
                              '
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
    subdomain_to_prop_value = 'fuel            ${D_hydraulic_fuel}
                               coupled_fuel    ${D_hydraulic_fuel}
                               fuel_orifice    ${D_hydraulic_fuel}
                               control         ${D_hydraulic_control}
                               control_orifice ${D_hydraulic_control}
                               cold_pool1      ${D_hydraulic_hotcoldpools}
                               cold_pool2      ${D_hydraulic_hotcoldpools}
                               hot_pool1       ${D_hydraulic_hotcoldpools}
                               hot_pool2       ${D_hydraulic_hotcoldpools}
                               inlet_duct      ${D_hydraulic_hotcoldpools}
                               free_duct       ${D_hydraulic_empty_duct}
                               cr_tip          ${D_hydraulic_empty_duct}
                              '
    block = ${flow_blocks}
  []

  [kappa_s]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'kappa_s'
    prop_values = '0 0 ${k_fuel}'
    block = ${rodded_flow_blocks}
  []

  # describe the drag coefficient in regions that have rods in them
  [drag_fuel]
    type = FunctorRehmeDragCoefficients
    multipliers = '100 100 1'
    hex_lattice = fuel_hex
    block = 'fuel coupled_fuel'
  []

  [drag_control]
    type = FunctorRehmeDragCoefficients
    multipliers = '100 100 1'
    hex_lattice = cr_hex
    block = 'control'
  []

  # orifice for the fuel assemblies
  [fuel_orifce_drag]
    type = ADGenericVectorFunctorMaterial
    prop_names =  'Darcy_coefficient Forchheimer_coefficient'
    prop_values = '1 1 1             10 10 10'
    block = 'fuel_orifice'
  []

  # the orifice contraining flow into the interwrapper space from
  # the cold pool
  [control_orifce_drag]
    type = ADGenericVectorFunctorMaterial
    prop_names =  'Darcy_coefficient Forchheimer_coefficient'
    prop_values = '1 1 1             ${control_Forchheimer} ${control_Forchheimer} ${control_Forchheimer}'
    block = 'control_orifice'
  []

  [pool_drag]
    type = ADGenericVectorFunctorMaterial
    prop_names =  'Darcy_coefficient Forchheimer_coefficient'
    prop_values = '1 1 1             10 10 10'
    block = ${pool_flow_blocks}
  []

  [empty_duct_drag]
    type = ADGenericVectorFunctorMaterial
    prop_names =  'Darcy_coefficient Forchheimer_coefficient'
    prop_values = '1 1 1             10 10 10'
    block = 'free_duct'
  []

  # use this to impose CR model
  [cr_tip_drag]
    type = ADGenericVectorFunctorMaterial
    prop_names =  'Darcy_coefficient Forchheimer_coefficient'
    prop_values = '1 1 1             10 10 10'
    block = 'cr_tip'
  []

  # alpha needs to be defined separately in the fuel and control
  # blocks because the geometry is different
  [alpha_fuel]
    type = FunctorHexagonalLatticeHTC
    hex_lattice = fuel_hex
    block = 'fuel coupled_fuel'
  []

  [alpha_control]
    type = FunctorHexagonalLatticeHTC
    hex_lattice = cr_hex
    block = 'control'
  []

  ## kappa_f is used instead of thermal conductivity k
  ## in the rodded regions it's different than k, otherwise it's k
  [kappa_f_fuel]
    type = FunctorLinearPecletHexagonalLatticeKappaFluid
    hex_lattice = fuel_hex
    block = 'fuel coupled_fuel'
  []

  [kappa_f_control]
    type = FunctorLinearPecletHexagonalLatticeKappaFluid
    hex_lattice = cr_hex
    block = 'control'
  []

  [free_kappa_f]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'kappa'
    prop_values = 'k k k'
    block = ${non_rodded_flow_blocks}
  []

[]

[Postprocessors]
  [total_heat]
    type = FunctionElementIntegral
    function = heat_source_fn
    block = 'fuel coupled_fuel'
  []

  [inlet_average_temperature]
    type = SideAverageValue
    boundary = inlet
    variable = T_fluid
  []

  [outlet_average_temperature]
    type = SideAverageValue
    boundary = outlet
    variable = T_fluid
  []

  [inlet_mfr]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho
    boundary = inlet
  []

  [outlet_mfr]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho
    boundary = outlet
  []

  [inlet_energy]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho_cp_T_fluid_var
    boundary = inlet
  []

  [outlet_energy]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho_cp_T_fluid_var
    boundary = outlet
  []

#  [interwrapper_mfr_in]
#    type = VolumetricFlowRate
#    vel_x = superficial_vel_x
#    vel_y = superficial_vel_y
#    vel_z = superficial_vel_z
#    advected_quantity = rho
#    boundary = c_pool_interwrapper
#  []
#
#  [interwrapper_mfr_out]
#    type = VolumetricFlowRate
#    vel_x = superficial_vel_x
#    vel_y = superficial_vel_y
#    vel_z = superficial_vel_z
#    advected_quantity = rho
#    boundary = h_pool_interwrapper
#  []
#
#  [control_el_mfr_in]
#    type = VolumetricFlowRate
#    vel_x = superficial_vel_x
#    vel_y = superficial_vel_y
#    vel_z = superficial_vel_z
#    advected_quantity = rho
#    boundary = c_pool_control_el
#  []
#
#  [control_el_mfr_out]
#    type = VolumetricFlowRate
#    vel_x = superficial_vel_x
#    vel_y = superficial_vel_y
#    vel_z = superficial_vel_z
#    advected_quantity = rho
#    boundary = h_pool_control_el
#  []
#
#  [fuel_el_mfr_in]
#    type = VolumetricFlowRate
#    vel_x = superficial_vel_x
#    vel_y = superficial_vel_y
#    vel_z = superficial_vel_z
#    advected_quantity = rho
#    boundary = c_pool_fuel_el
#  []
#
#  [fuel_el_mfr_out]
#    type = VolumetricFlowRate
#    vel_x = superficial_vel_x
#    vel_y = superficial_vel_y
#    vel_z = superficial_vel_z
#    advected_quantity = rho
#    boundary = h_pool_fuel_el
#  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       30                '

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.1
    iteration_window = 2
    optimal_iterations = 6
    growth_factor = 1.25
    cutback_factor = 0.8
  []

  nl_abs_tol = 1e-4
  end_time = 0.4 #1e5
  nl_max_its = 15
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
