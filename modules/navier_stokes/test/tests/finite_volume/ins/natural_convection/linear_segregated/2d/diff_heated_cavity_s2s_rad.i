################################################################################
# MATERIAL PROPERTIES
################################################################################
rho = 3279.
T_0 = 875.0
mu = 1.
k_cond = 38.0
cp = 640.
alpha = 3.26e-5

eps = 1.0

rad_left = 'left_0 left_1 left_2 left_3 left_4'
rad_right = 'right_0 right_1 right_2 right_3 right_4'
rad_top = 'top_0 top_1 top_2 top_3 top_4'
rad_bottom = 'bottom_0 bottom_1 bottom_2 bottom_3 bottom_4'

rad_fixed = '${rad_left} ${rad_right}'
rad_variable = '${rad_top} ${rad_bottom}'
rad_all = '${rad_left} ${rad_right} ${rad_top} ${rad_bottom}'
walls = ${rad_all}

[GlobalParams]
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  advected_interp_method = 'upwind'
  u = vel_x
  v = vel_y
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

################################################################################
# GEOMETRY
################################################################################

[Mesh]
  type = MeshGeneratorMesh
  parallel_type = replicated

  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 60
    ny = 60
  []

  [patch_left]
    type = PatchSidesetGenerator
    input = gen
    boundary = left
    n_patches = 5
    partitioner = centroid
    centroid_partitioner_direction = y
  []

  [patch_right]
    type = PatchSidesetGenerator
    input = patch_left
    boundary = right
    n_patches = 5
    partitioner = centroid
    centroid_partitioner_direction = y
  []

  [patch_top]
    type = PatchSidesetGenerator
    input = patch_right
    boundary = top
    n_patches = 5
    partitioner = centroid
    centroid_partitioner_direction = x
  []

  [patch_bottom]
    type = PatchSidesetGenerator
    input = patch_top
    boundary = bottom
    n_patches = 5
    partitioner = centroid
    centroid_partitioner_direction = x
  []
  [delete_original_sidesets]
    type = BoundaryDeletionGenerator
    input = patch_bottom
    boundary_names = 'left right top bottom'
  []
[]

################################################################################
# EQUATIONS: VARIABLES, KERNELS & BCS
################################################################################

[UserObjects]
  [ins_rhie_chow_interpolator]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
  []
[view_factor_study]
    type = ViewFactorRayStudy
    boundary = ${rad_all}
    execute_on = INITIAL

    face_order = FOURTH
    polar_quad_order = 12
    azimuthal_quad_order = 4
    warn_subdomain_hmax = false
  []

  [view_factor]
    type = RayTracingViewFactor
    boundary = ${rad_all}
    ray_study_name = view_factor_study
    normalize_view_factor = true
    execute_on = INITIAL
  []

  [gray_lambert]
    type = ViewFactorObjectSurfaceRadiation
    boundary = ${rad_all}

    fixed_temperature_boundary = ${rad_fixed}
    fixed_boundary_temperatures = '880 880 880 880 880
                                   870 870 870 870 870'

    # Top and bottom are variable-temperature radiating surfaces.
    adiabatic_boundary = ''

    emissivity = '${eps} ${eps} ${eps} ${eps} ${eps}
                  ${eps} ${eps} ${eps} ${eps} ${eps}
                  ${eps} ${eps} ${eps} ${eps} ${eps}
                  ${eps} ${eps} ${eps} ${eps} ${eps}'

    temperature = T_fluid
    view_factor_object_name = view_factor

    # NONLINEAR updates the radiation solution between SIMPLE iterations.
    execute_on = 'NONLINEAR'
  []
[]

[RayBCs]
  [view_factor]
    type = ViewFactorRayBC
    boundary = ${rad_all}
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = pressure_system
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = 875
  []
[]

[FVInterpolationMethods]
  [upwind]
    type = FVAdvectedUpwind
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method_name = upwind
    mu = ${mu}
    momentum_component = 'x'
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [u_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = vel_x
    T_fluid = T_fluid
    gravity = '0 -9.8 0'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = ${alpha}
    momentum_component = 'x'
  []

  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method_name = upwind
    mu = ${mu}
    momentum_component = 'y'
    use_nonorthogonal_correction = false
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []
  [v_buoyancy]
    type = LinearFVMomentumBoussinesq
    variable = vel_y
    T_fluid = T_fluid
    gravity = '0 -9.81 0'
    rho = ${rho}
    ref_temperature = ${T_0}
    alpha_name = ${alpha}
    momentum_component = 'y'
  []

  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = false
  []

   ####### FUEL ENERGY EQUATION #######

  [heat_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = temperature
    cp = ${cp}
  []
  [conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${fparse k_cond}
  []
[]


[LinearFVBCs]
  [no-slip-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_x
    boundary = ${walls}
    functor = 0
  []
  [no-slip-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = vel_y
    boundary = ${walls}
    functor = 0
  []
  [T_cold]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    boundary = ${rad_right}
    functor = 870.0
  []
  [T_hot]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    boundary = ${rad_left}
    functor = 880.0
  []

  [radiation_top_bottom]
    type = LinearFVGrayLambertBC
    variable = T_fluid
    temperature_radiation = T_fluid
    coeff_diffusion = ${k_cond}
    surface_radiation_object_name = gray_lambert
    boundary = ${rad_variable}
  []

  [pressure]
    type = LinearFVPressureFluxBC
    boundary = ${walls}
    variable = pressure
    HbyA_flux = HbyA
    Ainv = Ainv
    u = vel_x
    v = vel_y
    rho = ${rho}
  []
[]

[FunctorMaterials]
  [constant_functors]
    type = GenericFunctorMaterial
    prop_names = 'cp alpha_b'
    prop_values = '${cp} ${alpha}'
  []
[]

################################################################################
# EXECUTION / SOLVE
################################################################################

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-11
  pressure_l_abs_tol = 1e-11
  energy_l_abs_tol = 1e-11
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.9
  num_iterations = 1500
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  energy_absolute_tolerance = 1e-8
  print_fields = false
  momentum_l_max_its = 300

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.5 0.0 0.0'

  # momentum_petsc_options = '-ksp_monitor'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'

  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'

  continue_on_max_its = true
[]

################################################################################
# SIMULATION OUTPUTS
################################################################################

[Outputs]
  exodus = true
[]
