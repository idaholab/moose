# Full Step 6 segregated SIMPLE model with slip/symmetry walls.
#
# This is the linear-FV counterpart to step6newton.i.  The full Step 6 riser
# geometry and boundaries are retained, while the Pronghorn-only correlations in
# the Newton input are approximated with basic MOOSE functor materials.
p_out = 5.84e6
rho_f = 5.2955

bed_porosity = 0.39
bed_forch = 10.60
bottom_reflector_Dh = 0.1
riser_Dh = 0.17
c_drag_old = 10
clean_region_forch = 0

cp_f = 5200
k_f = 0.25
kappa_h = '${fparse k_f / cp_f}'
k_s = 26 #1e-3
k_s_br = '${fparse 0.7 * 26}'
# alpha = 2e4
pebble_diameter = 0.06

power_fn_scaling = 0.9792628
offset = -1.45819

riser_inner_radius = 1.701
riser_outer_radius = 1.871

mass_flow_rate = 64.3
flow_area = '${fparse pi * (riser_outer_radius * riser_outer_radius - riser_inner_radius * riser_inner_radius)}'
flow_vel = '${fparse mass_flow_rate / (flow_area * rho_f)}'

T_inlet = 533.25
h_inlet = '${fparse cp_f * T_inlet}'

advected_interp_method = 'upwind'

[Mesh]
  block_id = '1 2 3 4 5 6 7'
  block_name = 'pebble_bed
                cavity
                bottom_reflector
                side_reflector
                upper_plenum
                bottom_plenum
                riser'

  [cartesian_mesh]
    type = CartesianMeshGenerator
    dim = 2

    dx = '0.20 0.20 0.20 0.20 0.20 0.20
          0.010 0.055
          0.13
          0.102 0.102 0.102
          0.17
          0.120'

    ix = '1 1 1 1 1 1
          1 1
          1
          1 1 1
          2
          1'

    dy = '0.100 0.100
          0.967
          0.1709 0.1709 0.1709 0.1709 0.1709
          0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465
          0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465
          0.458 0.712'

    iy = '1 2
          2
          2 2 1 1 1
          4 1 1 1 1 1 1 1 1 1
          1 1 1 1 1 1 1 1 1 4
          4 2'

    subdomain_id = '4 4 4 4 4 4 4 4 4 4 4 4 4 4
                    4 4 4 4 4 4 4 4 4 4 4 4 4 4
                    6 6 6 6 6 6 6 6 6 4 4 4 7 4
                    3 3 3 3 3 3 4 4 4 4 4 4 7 4
                    3 3 3 3 3 3 4 4 4 4 4 4 7 4
                    3 3 3 3 3 3 4 4 4 4 4 4 7 4
                    3 3 3 3 3 3 4 4 4 4 4 4 7 4
                    3 3 3 3 3 3 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    1 1 1 1 1 1 4 4 4 4 4 4 7 4
                    2 2 2 2 2 2 5 5 5 5 5 5 7 4
                    4 4 4 4 4 4 4 4 4 4 4 4 4 4'
  []

  [inlet]
    type = SideSetsAroundSubdomainGenerator
    input = cartesian_mesh
    block = 7
    new_boundary = inlet
    normal = '0 -1 0'
  []

  [riser_top]
    type = SideSetsAroundSubdomainGenerator
    input = inlet
    block = 7
    new_boundary = riser_top
    normal = '0 1 0'
  []

  [riser_right]
    type = SideSetsAroundSubdomainGenerator
    input = riser_top
    block = 7
    new_boundary = riser_right
    normal = '1 0 0'
  []

  [riser_left]
    type = ParsedGenerateSideset
    input = riser_right
    combinatorial_geometry = 'abs(x-1.701) < 1e-3'
    included_subdomains = 7
    included_neighbors = 4
    new_sideset_name = riser_left
  []

  [upper_plenum_top]
    type = SideSetsAroundSubdomainGenerator
    input = riser_left
    block = 5
    new_boundary = upper_plenum_top
    normal = '0 1 0'
  []

  [upper_plenum_bottom]
    type = SideSetsAroundSubdomainGenerator
    input = upper_plenum_top
    block = 5
    new_boundary = upper_plenum_bottom
    normal = '0 -1 0'
  []

  [cavity_top]
    type = SideSetsAroundSubdomainGenerator
    input = upper_plenum_bottom
    block = 2
    new_boundary = cavity_top
    normal = '0 1 0'
  []

  [cavity_left]
    type = SideSetsAroundSubdomainGenerator
    input = cavity_top
    block = 2
    new_boundary = cavity_left
    normal = '-1 0 0'
  []

  [bed_left]
    type = SideSetsAroundSubdomainGenerator
    input = cavity_left
    block = 1
    new_boundary = bed_left
    normal = '-1 0 0'
  []

  [bed_right]
    type = SideSetsAroundSubdomainGenerator
    input = bed_left
    block = 1
    new_boundary = bed_right
    normal = '1 0 0'
  []

  [bottom_reflector_left]
    type = SideSetsAroundSubdomainGenerator
    input = bed_right
    block = 3
    new_boundary = bottom_reflector_left
    normal = '-1 0 0'
  []

  [bottom_reflector_right]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_reflector_left
    block = 3
    new_boundary = bottom_reflector_right
    normal = '1 0 0'
  []

  [bottom_plenum_left]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_reflector_right
    block = 6
    new_boundary = bottom_plenum_left
    normal = '-1 0 0'
  []

  [bottom_plenum_bottom]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_plenum_left
    block = 6
    new_boundary = bottom_plenum_bottom
    normal = '0 -1 0'
  []

  [bottom_plenum_top]
    type = ParsedGenerateSideset
    input = bottom_plenum_bottom
    combinatorial_geometry = 'abs(y-1.167) < 1e-3'
    included_subdomains = 6
    included_neighbors = 4
    new_sideset_name = bottom_plenum_top
  []

  [outlet]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_plenum_top
    block = 6
    new_boundary = outlet
    normal = '1 0 0'
  []

  [baffle_cav_up]
    type = SideSetsBetweenSubdomainsGenerator
    input = outlet
    primary_block = 2
    paired_block = 5
    new_boundary = baffle_cav_up
  []

  [baffle_cav_bed]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle_cav_up
    primary_block = 1
    paired_block = 2
    new_boundary = baffle_cav_bed
  []

  [bed_br_baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle_cav_bed
    primary_block = 1
    paired_block = 3
    new_boundary = bed_br_baffle
  []

  [br_bp_baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = bed_br_baffle
    primary_block = 3
    paired_block = 6
    new_boundary = br_bp_baffle
  []

  [riser_up_baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = br_bp_baffle
    primary_block = 7
    paired_block = 5
    new_boundary = riser_up_baffle
  []

  [DeleteBoundary]
    type = BoundaryDeletionGenerator
    input = riser_up_baffle
    boundary_names = 'left right top bottom'
  []

  [rename_boundaries]
    type = RenameBoundaryGenerator
    input = DeleteBoundary
    old_boundary = 'riser_right riser_top upper_plenum_top cavity_top cavity_left bed_left bottom_reflector_left bottom_plenum_left bottom_plenum_bottom riser_left upper_plenum_bottom bed_right bottom_reflector_right bottom_plenum_top'
    new_boundary = 'ex ex ex ex ex ex ex ex ex in in in in in'
  []

  coord_type = RZ
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system solid_energy_system'
  previous_nl_solution_required = true
[]

[FluidProperties]
  [fp]
    type = HeliumFluidProperties
  []
[]

[UserObjects]
  [rc]
    type = PorousRhieChowMassFlux
    u = superficial_u
    v = superficial_v
    pressure = pressure
    rho = 'rho_aux'
    # rho = 'rho'
    porosity = 'porosity'
    p_diffusion_kernel = p_diffusion
    pressure_baffle_sidesets = 'baffle_cav_bed baffle_cav_up bed_br_baffle br_bp_baffle riser_up_baffle'
    pressure_baffle_relaxation = 0.5

    reconstructed_pressure_gradient_feedback_relaxation = 1.0
    pressure_projection_method = consistent

    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'

    debug_baffle = false

    use_flux_velocity_reconstruction = true
    use_reconstructed_pressure_gradient = true
    flux_velocity_reconstruction_relaxation = 1.0
    flux_velocity_reconstruction_zero_flux_sidesets = 'ex in'
    # flux_velocity_reconstruction_zero_flux_sidesets = 'top_to_1 top_to_2 bottom_to_1 bottom_to_2'

    use_interpolated_density_in_bernoulli_jump = true
    use_corrected_pressure_gradient = false
  []
[]


[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.4
  pressure_variable_relaxation = 0.3
  num_iterations = 3000
  pressure_absolute_tolerance = 1e-7
  momentum_absolute_tolerance = '1e-7 1e-7'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = false

  energy_system = energy_system
  energy_l_abs_tol = 1e-12
  energy_l_tol = 0
  energy_equation_relaxation = 0.8
  energy_field_relaxation = 1.0
  energy_absolute_tolerance = 1e-7
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'

  solid_energy_system = solid_energy_system
  solid_energy_l_abs_tol = 1e-10
  solid_energy_l_tol = 0
  solid_energy_field_relaxation = 1.0
  solid_energy_absolute_tolerance = 1e-7
  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'
[]

[Variables]
  [superficial_u]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0.0
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []
  [superficial_v]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 1e-6
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = ${p_out}
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [h_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = ${h_inlet}
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    initial_condition = ${T_inlet}
    block = 'pebble_bed bottom_reflector side_reflector riser upper_plenum bottom_plenum'
  []


[]

[LinearFVKernels]


  [u_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_u
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = true
  []
  [v_advection]
    type = PorousLinearWCNSFVMomentumFlux
    variable = superficial_v
    advected_interp_method = ${advected_interp_method}
    mu = 'mu'
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    porosity_outside_divergence = true
    use_two_point_stress_transmissibility = true
  []
  [u_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_u
    momentum_component = 'x'
    rhie_chow_user_object = rc
    porosity = 'porosity'
    use_corrected_gradient = true
  []
  [v_pressure]
    type = LinearFVMomentumPressureUO
    variable = superficial_v
    momentum_component = 'y'
    rhie_chow_user_object = rc
    porosity = 'porosity'
    use_corrected_gradient = true
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusionJump
    variable = pressure
    diffusion_tensor = Ainv
    rhie_chow_user_object = rc
    use_nonorthogonal_correction = false
    debug_baffle_jump = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []

  [u_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_u
    Forchheimer_name = Forchheimer_coefficient
    Darcy_name = Darcy_coefficient
    porosity = porosity
    rho = rho_aux
    mu = mu
    # rho = rho
    u = superficial_u
    v = superficial_v
    momentum_component = 'x'
  []

  [v_friction]
    type = LinearFVMomentumPorousFriction
    variable = superficial_v
    Forchheimer_name = Forchheimer_coefficient
    Darcy_name = Darcy_coefficient
    porosity = porosity
    rho = rho_aux
    mu = mu
    # rho = rho
    u = superficial_u
    v = superficial_v
    momentum_component = 'y'
  []

  [fluid_energy_advection]
    type = LinearFVEnergyAdvection
    variable = h_fluid
    advected_quantity = enthalpy
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = rc
    subtract_continuity_error = true
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [fluid_energy_diffusion]
    type = LinearFVDiffusion
    variable = h_fluid
    # diffusion_coeff = kappa_h_var
    diffusion_coeff = kappa_h
    use_nonorthogonal_correction = false
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [fluid_solid_exchange]
    type = LinearFVEnthalpyVolumetricHeatTransfer
    variable = h_fluid
    h_solid_fluid = alpha
    cp = cp
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = false
    block = 'pebble_bed'
  []

  [solid_energy_diffusion]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = kappa_s
    use_nonorthogonal_correction = false
    block = 'pebble_bed bottom_reflector side_reflector riser upper_plenum bottom_plenum'
  []

  [source]
    type = LinearFVSource
    variable = T_solid
    source_density = heat_source_fn
    block = 'pebble_bed'
  []

  [convection_pebble_bed_fluid]
    type = LinearFVEnthalpyVolumetricHeatTransfer
    variable = T_solid
    h_solid_fluid = alpha
    cp = 1.0
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = true
    block = 'pebble_bed bottom_reflector'
  []

  [fluid_solid_exchange_br]
    type = LinearFVEnthalpyVolumetricHeatTransfer
    variable = h_fluid
    h_solid_fluid = alpha
    cp = cp
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = false
    block = 'bottom_reflector'
  []
[]

[LinearFVBCs]
  [inlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = inlet
    variable = superficial_u
    functor = 0
  []
  [inlet_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = inlet
    variable = superficial_v
    functor = ${flow_vel}
  []

  [inlet_T_fluid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet'
    variable = T_fluid
    functor = ${T_inlet}
  []

  [inlet_rho]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'inlet'
    variable = rho_aux
    functor = ${rho_f}
  []

  [pressure-extrapolation]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'inlet'
    variable = pressure
    functor = 0
  []

  # Fix the outlet pressure and leave the outlet velocity free.
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = outlet
    variable = superficial_u
    use_two_term_expansion = true
    assume_fully_developed_flow = true
    # assume_fully_developed_flow = true
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = outlet
    variable = superficial_v
    use_two_term_expansion = true
    assume_fully_developed_flow = true
    # assume_fully_developed_flow = true
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = outlet
    variable = pressure
    functor = ${p_out}
  []

  [symmetry_u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'ex in'
    variable = superficial_u
    u = superficial_u
    v = superficial_v
    momentum_component = x
  []
  [symmetry_v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'ex in'
    variable = superficial_v
    u = superficial_u
    v = superficial_v
    momentum_component = y
  []
  [pressure_symmetric]
    type = LinearFVPressureSymmetryBC
    boundary = 'ex in'
    variable = pressure
    HbyA_flux = 'HbyA'
  []


  [top_h_fluid]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = inlet
    variable = h_fluid
    # functor = h_from_p_T
    functor = ${h_inlet}
  []

  [side_h_fluid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'ex in'
    variable = h_fluid
    functor = 0.0
    # diffusion_coeff = kappa_h_var
    diffusion_coeff = kappa_h
  []

  [bottom_h_fluid]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = outlet
    variable = h_fluid
    use_two_term_expansion = false
  []

  [side_T_solid]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    boundary = 'ex'
    variable = T_solid
    functor = 0.0
    diffusion_coeff = kappa_s
  []




[]

[Functions]
  [heat_source_fn]
    type = ParsedFunction
    expression = '${power_fn_scaling} * (-1.0612e4 * pow(y+${offset}, 4) + 1.5963e5 * pow(y+${offset}, 3)
                   -6.2993e5 * pow(y+${offset}, 2) + 1.4199e6 * (y+${offset}) + 5.5402e4)'
  []

[]

[FunctorMaterials]

  [characteristic_length_mat]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = characteristic_length
    subdomain_to_prop_value = 'pebble_bed       ${pebble_diameter}
                               bottom_reflector ${bottom_reflector_Dh}
                               riser            ${riser_Dh}'
  []

  [fluid_props]
    type = GeneralFunctorFluidProps
    fp = fp
    pressure = pressure
    # Match step6newton.i: use inlet-temperature properties for the flow solve.
    T_fluid = ${T_inlet}
    speed = 1
    porosity = porosity
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
    characteristic_length = characteristic_length
  []

  [darcy_null]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'darcy_null'
    prop_values = '0 0 0'
  []

  [clean_region_darcy_scalar]
    type = ADParsedFunctorMaterial
    expression = '${c_drag_old} * rho_f / porosity / mu_f'
    property_name = clean_region_darcy
    functor_symbols = 'rho_f porosity mu_f'
    functor_names = 'rho porosity mu'
    block = 'cavity upper_plenum bottom_plenum riser'
  []

  [clean_region_darcy_vec]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'darcy_clean_vec'
    prop_values = 'clean_region_darcy clean_region_darcy clean_region_darcy'
    block = 'cavity upper_plenum bottom_plenum riser'
  []

  [darcy]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'Darcy_coefficient'
    subdomain_to_prop_value = 'pebble_bed darcy_null
                              cavity darcy_clean_vec
                              bottom_reflector darcy_null
                              upper_plenum darcy_clean_vec
                              bottom_plenum darcy_clean_vec
                              riser darcy_clean_vec'
  []

  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = 'pebble_bed       ${bed_porosity}
                               cavity           1
                               bottom_reflector 0.3
                               side_reflector   0
                               upper_plenum     0.2
                               bottom_plenum    0.2
                               riser            0.32'
  []

  [drag_bed]
    type = GenericVectorFunctorMaterial
    prop_names = 'bed_forch_vec'
    prop_values = '${bed_forch} ${bed_forch} ${bed_forch}'
  []

  [drag_cavity]
    type = GenericVectorFunctorMaterial
    prop_names = 'cavity_forch_vec'
    prop_values = '${clean_region_forch} ${clean_region_forch} ${clean_region_forch}'
  []

  [drag_bottom_reflector]
    type = GenericVectorFunctorMaterial
    prop_names = 'br_forch'
    prop_values = '270 0.027 270'    #  times porosity/2
  []

  [forch]
    type = PiecewiseByBlockVectorFunctorMaterial
    prop_name = 'Forchheimer_coefficient'
    subdomain_to_prop_value = 'pebble_bed bed_forch_vec
                              cavity cavity_forch_vec
                              bottom_reflector br_forch
                              upper_plenum cavity_forch_vec
                              bottom_plenum cavity_forch_vec
                              riser br_forch'
  []


  [fluid_enthalpy_material]
    type = LinearFVEnthalpyFunctorMaterial
    pressure = pressure
    T_fluid = T_fluid
    h = h_fluid
    h_from_p_T_functor = h_from_p_T_functor
    T_from_p_h_functor = T_from_p_h_functor
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [h_from_p_T_functor]
    type = ParsedFunctorMaterial
    property_name = h_from_p_T_functor
    functor_names = 'T_fluid'
    expression = '${cp_f} * T_fluid'
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [T_from_p_h_functor]
    type = ParsedFunctorMaterial
    property_name = T_from_p_h_functor
    functor_names = 'h_fluid'
    expression = 'h_fluid / ${cp_f}'
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [fluid_constants]
    type = GenericFunctorMaterial
    prop_names = 'cp_f kappa_h'
    prop_values = '${cp_f} ${kappa_h}'
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [kappa_h_var]
    type = ADParsedFunctorMaterial
    property_name = kappa_h_var
    functor_names = 'k cp'
    expression = 'k / cp'
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
    enable_jit = false
  []

  [solid_k_bed]
    type = GenericFunctorMaterial
    prop_names = 'kappa_s'
    prop_values = '${k_s}'
    block = 'pebble_bed'
  []

  [solid_k_br]
    type = GenericFunctorMaterial
    prop_names = 'kappa_s'
    prop_values = '${k_s_br}'
    block = 'bottom_reflector'
  []

  [solid_k_sr]
    type = GenericFunctorMaterial
    prop_names = 'kappa_s'
    prop_values = '${fparse 1 * 26}'
    block = 'side_reflector'
  []

  [solid_k_bottom_plenum]
    type = GenericFunctorMaterial
    prop_names = 'kappa_s'
    prop_values = '${fparse 0.8 * 26}'
    block = 'bottom_plenum'
  []

  [solid_k_upper_plenum]
    type = GenericFunctorMaterial
    prop_names = 'kappa_s'
    prop_values = '${fparse 0.8 * 26}'
    block = 'upper_plenum'
  []

  [solid_k_riser]
    type = GenericFunctorMaterial
    prop_names = 'kappa_s'
    prop_values = '${fparse 0.68 * 26}'
    block = 'riser'
  []

  [alpha_mat_bed]
    type = GenericFunctorMaterial
    prop_names = 'alpha'
    prop_values = '1.55e5'
    block = 'pebble_bed'
  []
  [alpha_mat_br]
    type = GenericFunctorMaterial
    prop_names = 'alpha'
    prop_values = '2e4'
    block = 'bottom_reflector'
  []
[]

[AuxVariables]
  [rho_aux]
    type = MooseLinearVariableFVReal
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []

  [porosity_aux]
    type = MooseLinearVariableFVReal
  []

  [T_fluid]
    type = MooseLinearVariableFVReal
    initial_condition = ${T_inlet}
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
  []


[]

[AuxKernels]
  [assign_rho_aux]
    type = FunctorAux
    variable = rho_aux
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
    functor = 'rho'
    execute_on = 'INITIAL NONLINEAR'
  []



  [assign_porosity_aux]
    type = FunctorAux
    variable = porosity_aux
    functor = 'porosity'
    execute_on = 'initial timestep_end'
  []

  [fluid_temperature]
    type = FunctorAux
    variable = T_fluid
    functor = T_from_p_h
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum riser'
    execute_on = 'INITIAL NONLINEAR'
  []

[]

[Postprocessors]
  [inlet_pressure]
    type = SideAverageValue
    variable = pressure
    boundary = inlet
    outputs = none
  []

  [outlet_pressure]
    type = SideAverageValue
    variable = pressure
    boundary = outlet
    outputs = none
  []

  [pressure_drop]
    type = ParsedPostprocessor
    pp_names = 'inlet_pressure outlet_pressure'
    expression = 'inlet_pressure - outlet_pressure'
  []

  [desired_mfr]
    type = Receiver
    default = ${mass_flow_rate}
  []

  [inlet_mfr]
    type = RhieChowMassFlowRate
    boundary = inlet
    rhie_chow_user_object = rc
  []
  [outlet_mfr]
    type = RhieChowMassFlowRate
    boundary = outlet
    rhie_chow_user_object = rc
  []

  [u_min]
    type = ElementExtremeValue
    variable = superficial_u
    value_type = min
    block = '1 2 3 5 6 7'
  []
  [u_max]
    type = ElementExtremeValue
    variable = superficial_u
    value_type = max
    block = '1 2 3 5 6 7'
  []

  [v_min]
    type = ElementExtremeValue
    variable = superficial_v
    value_type = min
    block = '1 2 3 5 6 7'
  []
  [v_max]
    type = ElementExtremeValue
    variable = superficial_v
    value_type = max
    block = '1 2 3 5 6 7'
  []

  [top_v_avg]
    type = SideAverageValue
    variable = superficial_v
    boundary = inlet
  []
  [bottom_v_avg]
    type = SideAverageValue
    variable = superficial_v
    boundary = outlet
  []

  [enthalpy_inlet]
    type = VolumetricFlowRate
    advected_quantity = h_fluid
    vel_x = superficial_u
    vel_y = superficial_v
    boundary = inlet
    rhie_chow_user_object = rc
  []

  [enthalpy_outlet]
    type = VolumetricFlowRate
    advected_quantity = h_fluid
    vel_x = superficial_u
    vel_y = superficial_v
    boundary = outlet
    rhie_chow_user_object = rc
    advected_interp_method = upwind
  []

  [heat_source_integral]
    type = ElementIntegralFunctorPostprocessor
    functor = heat_source_fn
    block = pebble_bed
  []

  [enthalpy_balance]
    type = ParsedPostprocessor
    pp_names = 'enthalpy_inlet enthalpy_outlet'
    expression = 'abs(enthalpy_outlet) - abs(enthalpy_inlet)'
  []

  [T_solid_max]
    type = ElementExtremeValue
    variable = T_solid
    value_type = max
    block = 'bottom_reflector pebble_bed riser bottom_plenum upper_plenum'
  []

  [T_outlet_avg]
    type = SideAverageValue
    variable = T_fluid
    boundary = outlet
    execute_on='INITIAL LINEAR TIMESTEP_END'
  []
[]



[Outputs]
  exodus = true
  csv = true
  console = true
  print_linear_residuals = true
  execute_on = 'TIMESTEP_END'
[]
