Pr_t = 0.9
rho = .01
mu = .01
cp = 1
k = 1

H = 10
bulk_u = 0.02

advected_interp_method = 'upwind'
pressure_tag = "pressure_grad"

### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Initial and Boundary Conditions ###
intensity = 0.05
k_init = '${fparse 1.5*(intensity * bulk_u)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / H}'

### Modeling parameters ###
bulk_wall_treatment = false
wall_treatment = 'neq' #Options: eq_newton, eq_incremental, eq_linearized, neq
#wall_treatment_T = 'neq'

fluid = '3'
solid = '2'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '.25 .25'
    dy = '1.0'
    ix = '5 5 '
    iy = '20'
    subdomain_id = '2 3'
  []
  [F2SolidWall]
    type = SideSetsBetweenSubdomainsGenerator
    new_boundary = 'F2SolidWall'
    primary_block = '2'
    paired_block= '3'
    input = mesh
  []
  [inlet-2]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x > .25 & y < .000001'
    new_sideset_name = inlet-2
    input = F2SolidWall
    replace = true
  []
  [outlet-2]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x > .25 & y > .99999'
    new_sideset_name = outlet-2
    input = inlet-2
    replace = true
  []
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system energy_system TKE_system TKED_system'
  previous_nl_solution_required = true
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  velocity_interp_method = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
    block = ${fluid}
  []
[]



[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0
    solver_sys = u_system
    two_term_boundary_expansion = false
    block = ${fluid}
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0
    solver_sys = v_system
    two_term_boundary_expansion = false
    block = ${fluid}
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 0
    solver_sys = pressure_system
    two_term_boundary_expansion = false
    block = ${fluid}
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = 800
    solver_sys = energy_system
    two_term_boundary_expansion = false
    block = ${fluid}
  []
  [T_solid]
    type = INSFVEnergyVariable
    initial_condition = 700
    solver_sys = energy_system
    two_term_boundary_expansion = false
    block = ${solid}
  []
  [TKE]
    type = INSFVEnergyVariable
    solver_sys = TKE_system
    initial_condition = '${fparse k_init}'
    block = ${fluid}
  []
  [TKED]
    type = INSFVEnergyVariable
    solver_sys = TKED_system
    initial_condition = '${eps_init}'
    block = ${fluid}
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_viscosity_turbulent]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu_t'
    momentum_component = 'x'
    complete_expansion = true
    u = vel_x
    v = vel_y
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_viscosity_turbulent]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu_t'
    momentum_component = 'y'
    complete_expansion = true
    u = vel_x
    v = vel_y
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []

  [p_diffusion]
    type = FVAnisotropicDiffusion
    variable = pressure
    coeff = "Ainv"
    coeff_interp_method = 'average'
  []
  [p_source]
    type = FVDivergence
    variable = pressure
    vector_field = "HbyA"
    force_boundary_execution = true
  []

  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
  []
  [temp_conduction]
    type = FVDiffusion
    coeff = ${k}
    variable = T_fluid
  []
  [temp_turb_conduction]
    type = FVDiffusion
    coeff = 'k_t'
    variable = T_fluid
  []

  [temp_solid_conduction]
    type = FVDiffusion
    coeff = ${k}
    variable = T_solid
  []
  [TKE_advection]
    type = INSFVTurbulentAdvection
    variable = TKE
    rho = ${rho}
  []
  [TKE_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKE
    coeff = ${mu}
  []
  [TKE_diffusion_turbulent]
    type = INSFVTurbulentDiffusion
    variable = TKE
    coeff = 'mu_t'
    scaling_coef = ${sigma_k}
  []
  [TKE_source_sink]
    type = INSFVTKESourceSink
    variable = TKE
    u = vel_x
    v = vel_y
    epsilon = TKED
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    walls = 'F2SolidWall right'
    wall_treatment = ${wall_treatment}
  []

  [TKED_advection]
    type = INSFVTurbulentAdvection
    variable = TKED
    rho = ${rho}
    walls = 'F2SolidWall right'
  []
  [TKED_diffusion]
    type = INSFVTurbulentDiffusion
    variable = TKED
    coeff = ${mu}
    walls = 'F2SolidWall right'
  []
  [TKED_diffusion_turbulent]
    type = INSFVTurbulentDiffusion
    variable = TKED
    coeff = 'mu_t'
    scaling_coef = ${sigma_eps}
    walls = 'F2SolidWall right'
  []
  [TKED_source_sink]
    type = INSFVTKEDSourceSink
    variable = TKED
    u = vel_x
    v = vel_y
    tke = TKE
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    C1_eps = ${C1_eps}
    C2_eps = ${C2_eps}
    walls = 'F2SolidWall right'
    wall_treatment = ${wall_treatment}
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'inlet-2'
    variable = vel_x
    functor = 0
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'inlet-2'
    variable = vel_y
    functor = 0.02
  []
  [inlet-temp]
    type = FVDirichletBC
    boundary = 'inlet-2'
    variable = T_fluid
    value = 800
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'F2SolidWall right'
    variable = vel_x
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'F2SolidWall right'
    variable = vel_y
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet-2'
    variable = pressure
    function = 0
  []
  [inlet_TKE]
    type = INSFVInletIntensityTKEBC
    boundary = 'inlet-2'
    variable = TKE
    u = vel_x
    v = vel_y
    intensity = ${intensity}
  []
  [inlet_TKED]
    type = INSFVMixingLengthTKEDBC
    boundary = 'inlet-2'
    variable = TKED
    tke = TKE
    characteristic_length = '${fparse 2*H}'
  []
  [walls_mu_t]
    type = INSFVTurbulentViscosityWallFunction
    boundary = 'right'
    variable = mu_t
    u = vel_x
    v = vel_y
    rho = ${rho}
    mu = ${mu}
    mu_t = 'mu_t'
    tke = TKE
    wall_treatment = ${wall_treatment}
  []
  [T_solid_wall]
    type = FVFunctorDirichletBC
    boundary = 'F2SolidWall'
    variable = T_solid
    functor = T_solid_wallBC
  []
[]

[FVInterfaceKernels]
  [NaK_Casing_convection]
    type = FVConvectionCorrelationInterface
    variable1 = T_fluid
    variable2 = T_solid
    boundary = 'F2SolidWall'
    h = '300'
    T_solid = T_solid
    T_fluid = T_fluid
    subdomain1 = 3
    subdomain2 = 2
    wall_cell_is_bulk = true
  []
[]

[AuxVariables]
  [mu_t]
    type = MooseVariableFVReal
    initial_condition = '${fparse 500 * C_mu * ${k_init}^2 / eps_init}'
    two_term_boundary_expansion = false
    block = ${fluid}
  []
  [k_t]
    type = MooseVariableFVReal
    initial_condition = 1.0
    block = ${fluid}
  []
  [yplus]
    type = MooseVariableFVReal
    two_term_boundary_expansion = false
    block = ${fluid}
  []
  [T_solid_wall]
    type = MooseVariableFVReal
    initial_condition = 800
    block = '2'
  []
[]

[AuxKernels]
  [compute_mu_t]
    type = kEpsilonViscosityAux
    variable = mu_t
    C_mu = ${C_mu}
    tke = TKE
    epsilon = TKED
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    bulk_wall_treatment = ${bulk_wall_treatment}
    walls = 'F2SolidWall right'
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
  []
  [compute_k_t]
    type = TurbulentConductivityAux
    variable = k_t
    Pr_t = ${Pr_t}
    cp = ${cp}
    mu_t = 'mu_t'
    execute_on = 'NONLINEAR'
  []
  [compute_y_plus]
    type = RANSYPlusAux
    variable = yplus
    tke = TKE
    mu = ${mu}
    rho = ${rho}
    u = vel_x
    v = vel_y
    walls = 'F2SolidWall right'
    wall_treatment = ${wall_treatment}
    execute_on = 'NONLINEAR'
  []
[]

[FunctorMaterials]
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    temperature = 'T_fluid'
    rho = ${rho}
    cp = ${cp}
  []
  [converter]
    type = FunctorADConverter
    ad_props_in = 'T_solid_wall'
    reg_props_out = 'T_solid_wallBC'
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  turbulence_systems = 'TKED_system TKE_system'

  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.5
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.5
  turbulence_equation_relaxation = '.3 .3'
  num_iterations = 10
  pressure_absolute_tolerance = 1e-8
  energy_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  turbulence_absolute_tolerance = '1e-8 1e-8'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold'
  momentum_petsc_options_value = 'hypre boomeramg .7'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type -pc_hypre_boomeramg_strong_threshold'
  pressure_petsc_options_value = 'hypre boomeramg .7'

  momentum_l_abs_tol = 1e-9
  pressure_l_abs_tol = 1e-9
  energy_l_abs_tol = 1e-9
  turbulence_l_abs_tol = 1e-9
  momentum_l_max_its = 1000
  pressure_l_max_its = 1000
  momentum_l_tol = 1e-9
  pressure_l_tol = 1e-9
  energy_l_tol = 1e-9
  turbulence_l_tol = 1e-9
  print_fields = false
  continue_on_max_its = true

  pin_pressure = false
[]

[Outputs]
  exodus= true
[]
