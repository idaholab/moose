mu=1.1
rho=1.1
velocity_interp_method='rc'

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
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = false
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1
  []
  [v]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1
  []
  [pressure]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

# [AuxVariables]
#   [v]
#     order = CONSTANT
#     family = MONOMIAL
#     fv = true
#   []
# []

# [ICs]
#   [v]
#     type = FunctionIC
#     function = 'exact_v'
#     variable = v
#   []
# []

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    velocity_interp_method = ${velocity_interp_method}
    vel = 'velocity'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
  []
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = 'velocity'
    advected_interp_method = 'average'
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
  []
  [u_pressure]
    # INSFVMomentumPressure inherits from FVMatAdvection and in INSFVMomentumPressure::validParams we set
    # 'advected_quantity = NS::pressure'
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'

    # these parameters shouldn't be used for anything but are still required
    vel = 'velocity'
    advected_interp_method = 'average'
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
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
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
    # these parameters shouldn't be used for anything but are still required
    vel = 'velocity'
    advected_interp_method = 'average'
  []
  [v_forcing]
    type = FVBodyForce
    variable = v
    function = forcing_v
  []
[]

[FVBCs]
  [u_advection]
    type = INSFVMomentumAdvectionFunctionBC
    boundary = 'left right top bottom'
    variable = u
    vel = 'velocity'
    flux_variable_exact_solution = 'exact_rhou'
    advected_quantity = 'rhou'
    advected_interp_method = 'average'
    vel_x_exact_solution = 'exact_u'
    vel_y_exact_solution = 'exact_v'
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    pressure_exact_solution = 'exact_p'
  []
  [u_diffusion]
    type = FVDiffusionFunctionBC
    boundary = 'left right top bottom'
    variable = u
    exact_solution = 'exact_u'
    coeff = '${mu}'
    coeff_function = '${mu}'
  []
  [u_pressure]
    type = INSFVMomentumPressureFunctionBC
    boundary = 'left right top bottom'
    variable = u
    momentum_component = 'x'
    p = pressure
    pressure_exact_solution = 'exact_p'
  []

  [v_advection]
    type = INSFVMomentumAdvectionFunctionBC
    boundary = 'left right top bottom'
    variable = v
    vel = 'velocity'
    flux_variable_exact_solution = 'exact_rhov'
    advected_quantity = 'rhov'
    advected_interp_method = 'average'
    vel_x_exact_solution = 'exact_u'
    vel_y_exact_solution = 'exact_v'
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    pressure_exact_solution = 'exact_p'
  []
  [v_diffusion]
    type = FVDiffusionFunctionBC
    boundary = 'left right top bottom'
    variable = v
    exact_solution = 'exact_v'
    coeff = '${mu}'
    coeff_function = '${mu}'
  []
  [v_pressure]
    type = INSFVMomentumPressureFunctionBC
    boundary = 'left right top bottom'
    variable = v
    momentum_component = 'y'
    p = pressure
    pressure_exact_solution = 'exact_p'
  []

  [mass_continuity_flux]
    type = INSFVMomentumAdvectionFunctionBC
    variable = pressure
    boundary = 'top bottom left right'
    vel = 'velocity'
    flux_variable_exact_solution = 'rho'
    advected_quantity = ${rho}
    vel_x_exact_solution = 'exact_u'
    vel_y_exact_solution = 'exact_v'
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    # WTF, this errors with:
    #    Material property '${rho}', requested by
    #    'mass_continuity_flux' is not defined on boundary bottom
    # if I try to do dollar bracket substitution, so I'm just gonna write it out
    rho = 1.1
    pressure_exact_solution = 'exact_p'
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
  [p_diri]
    type = FVFunctionDirichletBC
    variable = pressure
    boundary = 'top bottom left right'
    function = 'exact_p'
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    # we need to compute this here for advection in INSFVMomentumPressure
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
  value = '1.331*mu*sin(1.1*x) - 0.891*rho*sin(1.1*x)*sin(0.9*y) + 2.662*rho*sin(1.1*x)*cos(1.1*x) + 0.96*cos(0.8*x)*cos(1.3*y)'
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
  value = '0.729*mu*cos(0.9*y) - 1.458*rho*sin(0.9*y)*cos(0.9*y) + 1.089*rho*cos(1.1*x)*cos(0.9*y) - 1.56*sin(0.8*x)*sin(1.3*y)'
  vars = 'mu rho'
  vals = '${mu} ${rho}'
[]
[exact_p]
  type = ParsedFunction
  value = '1.2*sin(0.8*x)*cos(1.3*y)'
[]
[forcing_p]
  type = ParsedFunction
  value = '-0.81*rho*sin(0.9*y) + 1.21*rho*cos(1.1*x)'
  vars = 'rho'
  vals = '${rho}'
[]
[rho]
  type = ParsedFunction
  value = '${rho}'
[]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      200                lu           NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
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
