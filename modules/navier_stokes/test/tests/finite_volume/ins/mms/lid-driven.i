mu=1.1
rho=1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.6
    xmax = 0.6
    ymin = -0.6
    ymax = 0.6
    nx = 2
    ny = 2
  []
  parallel_type = distributed
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = false
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
    two_term_boundary_expansion = true
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1
    two_term_boundary_expansion = true
  []
  [pressure]
    type = INSFVPressureVariable
    two_term_boundary_expansion = true
  []
  [lambda]
    order = FIRST
    family = SCALAR
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = 'average'
    velocity_interp_method = 'average'
    vel = 'velocity'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    no_slip_wall_boundaries = 'left right top bottom'
  []
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
  []
  [mean_zero_pressure]
    type = FVScalarLagrangeMultiplier
    variable = pressure
    lambda = lambda
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = 'velocity'
    advected_interp_method = 'average'
    velocity_interp_method = 'average'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    no_slip_wall_boundaries = 'left right top bottom'
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    p = pressure
  []
  [u_forcing]
    type = FVBodyForce
    variable = u
    function = forcing_u
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_quantity = 'rhov'
    vel = 'velocity'
    advected_interp_method = 'average'
    velocity_interp_method = 'average'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    no_slip_wall_boundaries = 'left right top bottom'
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = ${mu}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    p = pressure
  []
  [v_forcing]
    type = FVBodyForce
    variable = v
    function = forcing_v
  []
[]

[FVBCs]
  [u_advection]
    type = FVMatAdvectionFunctionBC
    boundary = 'left right top bottom'
    variable = u
    vel = 'velocity'
    flux_variable_exact_solution = 'exact_rhou'
    advected_quantity = 'rhou'
    advected_interp_method = 'average'
    vel_x_exact_solution = 'exact_u'
    vel_y_exact_solution = 'exact_v'
  []
  [u_diffusion]
    type = FVDiffusionFunctionBC
    boundary = 'left right top bottom'
    variable = u
    exact_solution = 'exact_u'
    coeff = '${mu}'
    coeff_function = '${mu}'
  []

  [v_advection]
    type = FVMatAdvectionFunctionBC
    boundary = 'left right top bottom'
    variable = v
    vel = 'velocity'
    flux_variable_exact_solution = 'exact_rhov'
    advected_quantity = 'rhov'
    advected_interp_method = 'average'
    vel_x_exact_solution = 'exact_u'
    vel_y_exact_solution = 'exact_v'
  []
  [v_diffusion]
    type = FVDiffusionFunctionBC
    boundary = 'left right top bottom'
    variable = v
    exact_solution = 'exact_v'
    coeff = '${mu}'
    coeff_function = '${mu}'
  []

  [mass_continuity_flux]
    type = FVMatAdvectionFunctionBC
    variable = pressure
    boundary = 'top bottom left right'
    vel = 'velocity'
    vel_x_exact_solution = 'exact_u'
    vel_y_exact_solution = 'exact_v'
    flux_variable_exact_solution = ${rho}
    advected_quantity = ${rho}
  []

  [u_diri]
    type = FVFunctionDirichletBC
    variable = u
    boundary = 'top bottom left right'
    function = 'exact_u'
  []
  [v_diri]
    type = FVFunctionDirichletBC
    variable = v
    boundary = 'top bottom left right'
    function = 'exact_v'
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    rho = ${rho}
  []
[]

[Functions]
[exact_u]
  type = ParsedFunction
  value = '1.1*sin(1.1*x)'
[]
[exact_rhou]
  type = ParsedFunction
  value = '1.1*rho*sin(1.1*x)'
  vars = 'rho'
  vals = '${rho}'
[]
[forcing_u]
  type = ParsedFunction
  value = '1.331*mu*sin(1.1*x) - 0.891*rho*sin(1.1*x)*sin(0.9*y) + 2.662*rho*sin(1.1*x)*cos(1.1*x) + 0.96*cos(0.8*x)*sin(1.3*y)'
  vars = 'mu rho'
  vals = '${mu} ${rho}'
[]
[exact_v]
  type = ParsedFunction
  value = '0.9*cos(0.9*y)'
[]
[exact_rhov]
  type = ParsedFunction
  value = '0.9*rho*cos(0.9*y)'
  vars = 'rho'
  vals = '${rho}'
[]
[forcing_v]
  type = ParsedFunction
  value = '0.729*mu*cos(0.9*y) - 1.458*rho*sin(0.9*y)*cos(0.9*y) + 1.089*rho*cos(1.1*x)*cos(0.9*y) + 1.56*sin(0.8*x)*cos(1.3*y)'
  vars = 'mu rho'
  vals = '${mu} ${rho}'
[]
[exact_p]
  type = ParsedFunction
  value = '1.2*sin(0.8*x)*sin(1.3*y)'
[]
[forcing_p]
  type = ParsedFunction
  value = '-0.81*rho*sin(0.9*y) + 1.21*rho*cos(1.1*x)'
  vars = 'rho'
  vals = '${rho}'
[]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart -pc_asm_overlap'
  petsc_options_value = 'asm      lu           NONZERO                   100                4'
[]

[Outputs]
  csv = true
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [./L2u]
    type = ElementL2Error
    variable = u
    function = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
  [./L2v]
    variable = v
    function = exact_v
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
  [./L2p]
    variable = pressure
    function = exact_p
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
[]
