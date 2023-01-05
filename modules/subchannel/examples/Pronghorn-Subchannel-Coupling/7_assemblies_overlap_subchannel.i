# This model drives a multiapp setup where wrappers (solid heat conduction)
# and interwrapper flow (fluid) are solved in this input. Intra-element flow is solved
# by subchannel
# heat is applied in the pin model in subchannel

# Units are SI

# physics parameters
inlet_temperature = 628.15
outlet_pressure = 0 # gauge pressure

# geometry parameters
pin_diameter = 8e-3
pin_pitch = 9.04e-3
# cr_pin_diameter = 1.11e-2
# cr_pin_pitch = 1.2476e-2
flat_to_flat = 13.598e-2
wire_diameter = 1.03e-3
wire_pitch = 203.2e-3
inter_assembly_gap = 4e-3

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

[Modules]
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
    x = '0     1'
    y = '1e-15 0.1'
  []

  [inlet_vel_assembly_fn]
    type = PiecewiseLinear
    x = '0     1'
    y = '1e-15 0.5'
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
    block = 'porous_flow center_porous_flow'
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
  [inlet_average_temperature]
    type = SideAverageValue
    boundary = 'inlet_assembly inlet_interwrapper'
    variable = T_fluid
  []

  [outlet_average_temperature]
    type = SideAverageValue
    boundary = outlet
    variable = T_fluid
  []

  [outlet_average_pressure]
    type = SideAverageValue
    boundary = outlet_central_assembly
    variable = pressure
  []

  [inlet_mfr]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho
    boundary = 'inlet_assembly inlet_interwrapper'
  []

  [inlet_central_mfr]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho
    boundary = 'inlet_central_assembly'
  []

  [outlet_mfr]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho
    boundary = outlet
  []

  [central_assembly_area]
    type = AreaPostprocessor
    boundary = inlet_central_assembly
  []

  [inlet_mass_flux]
    type = ParsedPostprocessor
    pp_names = 'inlet_central_mfr central_assembly_area'
    function = 'abs(inlet_central_mfr/central_assembly_area)'
  []

  [inlet_energy]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho_cp_T_fluid_var
    boundary = 'inlet_assembly inlet_interwrapper'
  []

  [outlet_energy]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = rho_cp_T_fluid_var
    boundary = outlet
  []

  [report_pressure_drop]
    type = Receiver
  []
[]

# [Executioner]
#   type = Transient
#   solve_type = 'NEWTON'
#   petsc_options_iname = '-pc_type -ksp_gmres_restart -pc_factor_shift_type -pc_factor_shift_amount'
#   petsc_options_value = 'lu       30                 NONZERO               1e-10   '
#   dt = 1
#   end_time = 10
#   #dt = 0.1
#   #end_time = 1e5
#   #[TimeStepper]
#   #  type = IterationAdaptiveDT
#   #  dt = 0.1
#   #  iteration_window = 2
#   #  optimal_iterations = 6
#   #  growth_factor = 1.25
#   #  cutback_factor = 0.8
#   #[]

#   nl_abs_tol = 1e-4
#   nl_max_its = 30
# []

# [Executioner]
#   type = Steady
#   petsc_options_iname = '-pc_type -pc_hypre_type'
#   petsc_options_value = 'hypre boomeramg'
#   fixed_point_max_its = 2
#   fixed_point_min_its = 1
#   fixed_point_rel_tol = 1e-6
# []

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  line_search = 'none'
[]

[Outputs]
  exodus = true
  #print_linear_residuals = false
[]

# the only reason this multiapp is here because I cannot fucking
# get the T_wrapper monomial variable to evaluate the fucking qdot
# correctly. I can also not get it into a Lagrange variable either.
# [MultiApps]
#   [dummy]
#     type = TransientMultiApp
#     input_files = 'dummy.i'
#     execute_on = 'TIMESTEP_END'
#   []
# []

# [Transfers]
#   [send]
#     type = MultiAppProjectionTransfer
#     to_multi_app = dummy
#     variable = T_wrapper
#     source_variable = T_wrapper
#   []

#   [receive]
#     type = MultiAppProjectionTransfer
#     from_multi_app = dummy
#     variable = T_wrapper_linear
#     source_variable = T_wrapper
#   []
# []
