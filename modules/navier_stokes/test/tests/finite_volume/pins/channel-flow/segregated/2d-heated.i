mu = 1
rho = 1
k = 1e-3
cp = 1
u_inlet = 1
T_inlet = 200
advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

pressure_tag = "pressure_grad"

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5 5'
    dy = '1.0'
    ix = '10 10'
    iy = '5'
    subdomain_id = '1 2'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolatorSegregated
    u = superficial_vel_x
    v = superficial_vel_y
    pressure = pressure
    porosity = porosity
  []
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system energy_system solid_energy_system'
  previous_nl_solution_required = true
[]

[Variables]
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = ${u_inlet}
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    two_term_boundary_expansion = false
    solver_sys = pressure_system
  []
  [T_fluid]
    type = INSFVEnergyVariable
    two_term_boundary_expansion = false
    solver_sys = energy_system
    initial_condition = 200
  []
  [T_solid]
    type = MooseVariableFVReal
    two_term_boundary_expansion = false
    solver_sys = solid_energy_system
    initial_condition = 200
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  [u_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_x
    mu = ${mu}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_x
    momentum_component = 'x'
    pressure = pressure
    porosity = porosity
    extra_vector_tags = ${pressure_tag}
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_y
    mu = ${mu}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_y
    momentum_component = 'y'
    pressure = pressure
    porosity = porosity
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

  [energy_advection]
    type = PINSFVEnergyAdvection
    variable = T_fluid
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    boundaries_to_force = bottom
  []
  [energy_diffusion]
    type = PINSFVEnergyDiffusion
    k = ${k}
    variable = T_fluid
    porosity = porosity
  []
  [energy_convection]
    type = PINSFVEnergyAmbientConvection
    variable = T_fluid
    is_solid = false
    T_fluid = 'T_fluid'
    T_solid = 'T_solid'
    h_solid_fluid = 'h_cv'
  []

  [solid_energy_diffusion]
    type = FVDiffusion
    coeff = ${k}
    variable = T_solid
  []
  [solid_energy_convection]
    type = PINSFVEnergyAmbientConvection
    variable = T_solid
    is_solid = true
    T_fluid = 'T_fluid'
    T_solid = 'T_solid'
    h_solid_fluid = 'h_cv'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = superficial_vel_x
    function = ${u_inlet}
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = superficial_vel_y
    function = 0
  []
  [inlet-T]
    type = FVDirichletBC
    variable = T_fluid
    value = ${T_inlet}
    boundary = 'left'
  []

  [no-slip-u]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = superficial_vel_x
    function = 0
  []
  [no-slip-v]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = superficial_vel_y
    function = 0
  []
  [heated-side]
    type = FVDirichletBC
    boundary = 'top'
    variable = 'T_solid'
    value = 250
  []

  [symmetry-u]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = superficial_vel_x
    u = superficial_vel_x
    v = superficial_vel_y
    mu = ${mu}
    momentum_component = 'x'
  []
  [symmetry-v]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = superficial_vel_y
    u = superficial_vel_x
    v = superficial_vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [symmetry-p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []

  [outlet-p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0.1
  []
[]

[FunctorMaterials]
  [constants]
    type = ADGenericFunctorMaterial
    prop_names = 'h_cv cp'
    prop_values = '0.1 ${cp}'
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    rho = ${rho}
    temperature = 'T_fluid'
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  energy_l_abs_tol = 1e-14
  solid_energy_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  solid_energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  solid_energy_system = 'solid_energy_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.4
  energy_equation_relaxation = 1.0
  num_iterations = 160
  pressure_absolute_tolerance = 1e-12
  momentum_absolute_tolerance = 1e-12
  energy_absolute_tolerance = 1e-12
  solid_energy_absolute_tolerance = 1e-12
  print_fields = false
[]

[Outputs]
  exodus = true
  csv = false
[]
