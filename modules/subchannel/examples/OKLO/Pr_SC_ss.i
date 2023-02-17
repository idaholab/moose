# This model drives a multiapp setup where wrappers (solid heat conduction)
# and interwrapper flow (fluid) and interwall flow (fluid) are solved in this input.
# by subchannel.

# Units are SI

# physics parameters
inlet_temperature = 624.70556
outlet_pressure = 2.0e5 # Pa # gauge pressure

# geometry parameters
###################################################
# Geometric parameters of XX09
###################################################
# units are cm - do not forget to convert to meter
scale_factor = 0.01
pin_pitch = ${fparse 0.5664*scale_factor}
pin_diameter = ${fparse 0.4419*scale_factor}
wire_pitch = ${fparse 15.24*scale_factor}
wire_diameter = ${fparse 0.1244*scale_factor}
flat_to_flat = ${fparse 4.64*scale_factor}
n_rings = 5
heated_length = ${fparse 34.3*scale_factor}
unheated_length_exit = ${fparse 26.9*scale_factor}
length = ${fparse heated_length + unheated_length_exit}
inter_wrapper_width = ${fparse 0.3*scale_factor}
outer_duct_in = ${fparse 5.5854*scale_factor}
inner_duct_out =  ${fparse 4.8437*scale_factor}
###################################################
inter_wall_width = ${fparse outer_duct_in - inner_duct_out}

# fluid properties
####  Density #####
A12 = 1.00423e3
A13 = -0.21390
A14 = -1.1046e-5
rho = ${fparse A12 + A13 * inlet_temperature + A14 * inlet_temperature * inlet_temperature}
#### Viscosity
A52 = 3.6522e-5
A53 = 0.16626
A54 = -4.56877e1
A55 = 2.8733e4
mu = ${fparse A52 + A53 / inlet_temperature + A54 / inlet_temperature / inlet_temperature +
        A55 / (inlet_temperature * inlet_temperature * inlet_temperature)}
#### Specific heat at constant pressure
A28 = 7.3898e5
A29 = 3.154e5
A30 = 1.1340e3
A31 = -2.2153e-1
A32 = 1.1156e-4
dt = ${fparse 2503.3 - inlet_temperature}
cp = ${fparse A28 / dt / dt + A29 / dt + A30 + A31 * dt + A32 * dt * dt}
#### Heat conduction coefficient
A48 = 1.1045e2
A49 = -6.5112e-2
A50 = 1.5430e-5
A51 = -2.4617e-9
k = ${fparse A48 + A49 * inlet_temperature + A50 * inlet_temperature * inlet_temperature +
        A51 * inlet_temperature * inlet_temperature * inlet_temperature}
#### Molar mass
molar_mass = 22.989769e-3

# wrapper properties
k_wrapper = 20
cp_wrapper = 300
rho_wrapper = 7800

# hydraulic diameters
D_hydraulic_interwrapper = ${fparse 2 * inter_wrapper_width}
D_hydraulic_interwall = ${fparse 2 * inter_wall_width}
D_hydraulic_fuel = 0.00297 # Why?

wrapper_blocks = 'wall'
flow_blocks = 'inter_wrapper interwall center_porous_flow
               porous_flow_hfd porous_flow_p porous_flow_d1 porous_flow_d2 porous_flow_k011 porous_flow_x402'
porous_flow = 'porous_flow_hfd porous_flow_p porous_flow_d1 porous_flow_d2 porous_flow_k011 porous_flow_x402'

[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = 'EBRT-ii_7assemblies_in.e'
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
  [XX09_hex]
    type = HexagonalLattice
    flow_region_flat_to_flat = ${flat_to_flat}
    pin_pitch = ${pin_pitch}
    pin_diameter = ${pin_diameter}
    wire_diameter = ${wire_diameter}
    wire_pitch = ${wire_pitch}
    n_rings = 5
  []
  [DRIVER_hex]
    type = HexagonalLattice
    flow_region_flat_to_flat = 0.056134
    pin_pitch = ${pin_pitch}
    pin_diameter = 0.0044196
    wire_diameter = ${wire_diameter}
    wire_pitch = 0.1524
    n_rings = 6
  []
  [SST_hex]
    type = HexagonalLattice
    flow_region_flat_to_flat = 0.056134
    pin_pitch = 0.02059 #
    pin_diameter = 0.02046 #m
    wire_diameter = 0.0
    n_rings = 2
  []
[]

[Functions]
  [mu_rampdown_fn]
    type = PiecewiseLinear
    x = '10'
    y = ${mu}
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
  [T_wrapper_inside_wall]
    type = FVFunctorDirichletBC
    variable = T_wrapper
    functor = duct_surface_temperature_functor
    boundary = 'inner_wall_in'
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
    boundary = 'inner_wall_in'
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
    thermal_conductivity_blocks = 'inter_wrapper; ${porous_flow}; center_porous_flow'
    specific_heat = 'cp'

    # initial conditions
    initial_velocity = '0 0 1e-4'
    initial_temperature = '${inlet_temperature}'
    initial_pressure = '${outlet_pressure}'

    # boundary conditions
    inlet_boundaries =     'inlet_interwrapper
                            inlet_interwall
                            inlet_central_assembly
                            inlet_porous_flow_hfd
                            inlet_porous_flow_p
                            inlet_porous_flow_d1
                            inlet_porous_flow_d2
                            inlet_porous_flow_k011
                            inlet_porous_flow_x402'

    momentum_inlet_types = 'flux-mass
                            flux-mass
                            flux-mass
                            flux-mass
                            flux-mass
                            flux-mass
                            flux-mass
                            flux-mass
                            flux-mass'

    flux_inlet_pps  = 'interwall_mass_flux
                       interwall_mass_flux
                       XX09_mass_flux
                       HFD_mass_flux
                       P_mass_flux
                       D1_mass_flux
                       D2_mass_flux
                       K011_mass_flux
                       X402_mass_flux'

    energy_inlet_types = 'fixed-temperature
                          fixed-temperature
                          fixed-temperature
                          fixed-temperature
                          fixed-temperature
                          fixed-temperature
                          fixed-temperature
                          fixed-temperature
                          fixed-temperature'

    energy_inlet_function = '${inlet_temperature}
                             ${inlet_temperature}
                             ${inlet_temperature}
                             ${inlet_temperature}
                             ${inlet_temperature}
                             ${inlet_temperature}
                             ${inlet_temperature}
                             ${inlet_temperature}
                             ${inlet_temperature}'

    wall_boundaries = 'inner_wall_in inner_wall_out wall_in wall_out'
    momentum_wall_types = 'slip slip slip slip'
    energy_wall_types = 'heatflux heatflux heatflux heatflux'
    energy_wall_function = '0 0 0 0'

    outlet_boundaries = 'outlet_interwrapper outlet_interwall outlet_central_assembly outlet_porous_flow'
    momentum_outlet_types = 'fixed-pressure fixed-pressure fixed-pressure fixed-pressure'
    pressure_function = '${outlet_pressure} ${outlet_pressure} ${outlet_pressure} ${outlet_pressure}'
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
    block = '${porous_flow} center_porous_flow interwall'
  []

  [fluid_props_interwrapper]
    type = GeneralFunctorFluidProps
    T_fluid = T_fluid
    pressure = pressure
    porosity = porosity
    speed = speed
    fp = fp
    characteristic_length = ${D_hydraulic_interwrapper}
    block = 'inter_wrapper'
  []

  ## set the blockwise porosity
  [porosity_functor_mat]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'porosity'
    subdomain_to_prop_value = 'inter_wrapper 1
                               interwall     1
                               porous_flow_hfd  0.447848479
                               porous_flow_p   0.447848479
                               porous_flow_d1  0.447848479
                               porous_flow_d2  0.447848479
                               porous_flow_k011  0.156633069
                               porous_flow_x402  0.447848479
                               center_porous_flow 0.45846977'
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
    subdomain_to_prop_value = 'inter_wrapper ${D_hydraulic_interwrapper}
                               interwall     ${D_hydraulic_interwall}
                               porous_flow_hfd  ${D_hydraulic_fuel}
                               porous_flow_p  ${D_hydraulic_fuel}
                               porous_flow_d1  ${D_hydraulic_fuel}
                               porous_flow_d2  ${D_hydraulic_fuel}
                               porous_flow_k011  ${D_hydraulic_fuel}
                               porous_flow_x402  ${D_hydraulic_fuel}
                               center_porous_flow ${D_hydraulic_fuel}'
    block = ${flow_blocks}
  []

  # Flow resistances
  [interwrapper_drag]
    type = FunctorChurchillDragCoefficients
    block = 'inter_wrapper interwall'
  []

  [Driver_drag]
    type = FunctorRehmeDragCoefficients
    multipliers = '100 100 1'
    hex_lattice = DRIVER_hex
    block = 'porous_flow_d1 porous_flow_d2 porous_flow_hfd porous_flow_p porous_flow_x402'
  []

  [SST_drag]
    type = FunctorRehmeDragCoefficients
    multipliers = '100 100 1'
    hex_lattice = SST_hex
    block = 'porous_flow_k011'
  []

  [center_assembly_drag]
    type = FunctorRehmeDragCoefficientsPressureGradient
    multipliers = '100 100 1'
    hex_lattice = XX09_hex
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
    # block = ${wrapper_blocks}
    initial_condition = 0
  []

  [T_wrapper_linear]
    # block = ${wrapper_blocks}
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
    diffusion_variable = T_wrapper_linear
    component = normal
    boundary = 'inner_wall_in'
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

  [interwall_area]
    type = AreaPostprocessor
    boundary = inlet_interwall
  []

  [inter_wrapper_area]
    type = AreaPostprocessor
    boundary = inlet_interwrapper
  []

  [XX09_area]
    type = AreaPostprocessor
    boundary = inlet_central_assembly
  []

  [P_area]
    type = AreaPostprocessor
    boundary = inlet_porous_flow_p
  []

  [hfd_area]
    type = AreaPostprocessor
    boundary = inlet_porous_flow_hfd
  []

  [D1_area]
    type = AreaPostprocessor
    boundary = inlet_porous_flow_d1
  []

  [D2_area]
    type = AreaPostprocessor
    boundary = inlet_porous_flow_d2
  []

  [K011_area]
    type = AreaPostprocessor
    boundary = inlet_porous_flow_k011
  []

  [X402_area]
    type = AreaPostprocessor
    boundary = inlet_porous_flow_x402
  []

  ##########################################
  [interwall_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'interwall_area'
    function = 'abs(0.2423/interwall_area)'
  []

  [XX09_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'XX09_area'
    function = 'abs(2.45/XX09_area)'
  []

  [D1_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'D1_area'
    function = 'abs(4.62/D1_area)'
  []

  [D2_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'D2_area'
    function = 'abs(4.618/D2_area)'
  []

  [K011_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'K011_area'
    function = 'abs(0.63136/K011_area)'
  []

  [X402_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'X402_area'
    function = 'abs(3.5264/X402_area)'
  []

  [HFD_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'hfd_area'
    function = 'abs(3.5264/hfd_area)'
  []

  [P_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'P_area'
    function = 'abs(2.916/P_area)'
  []

  ###########################################

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
  end_time = 0.04
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

# [MultiApps]
#   [subchannel]
#     # app_type = SubchannelApp
#     type = FullSolveMultiApp
#     input_files = 'subchannel.i'
#     execute_on = 'timestep_end'
#     positions = '0 0 0'
#     max_procs_per_app = 1
#     output_in_position = true
#     bounding_box_padding = '0 0 0.1'
#   []
# []

# [Transfers]
#   [pressure_drop_transfer] # Get pressure drop to pronghorn from subchannel
#     type = MultiAppPostprocessorTransfer
#     from_multi_app =  subchannel
#     from_postprocessor = total_pressure_drop_SC
#     to_postprocessor = report_pressure_drop
#     reduction_type = average
#     execute_on = 'timestep_end'
#   []

#   [mass_flux_tranfer] # Send mass_flux at the inlet to subchannel
#     type = MultiAppPostprocessorTransfer
#     to_multi_app = subchannel
#     from_postprocessor = inlet_mass_flux_Pr
#     to_postprocessor = report_mass_flux_inlet
#     execute_on = 'timestep_end'
#   []

#   [outlet_pressure_tranfer] # Send pressure at the outlet to subchannel
#     type = MultiAppPostprocessorTransfer
#     to_multi_app = subchannel
#     from_postprocessor = outlet_average_pressure_Pr
#     to_postprocessor = report_pressure_outlet
#     execute_on = 'timestep_end'
#   []
# []
