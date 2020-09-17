mu=1.1
rho=1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  []
[]

[Problem]
  kernel_coverage_check = false
  coord_type = 'RZ'
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

[FVKernels]
  [mass]
    type = NSFVKernel
    variable = pressure
    advected_quantity = 1
    advected_interp_method = 'average'
    velocity_interp_method = 'rc'
    vel = 'velocity'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    ghost_layers = 2
  []
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
  []

  [u_advection]
    type = NSFVKernel
    variable = u
    advected_quantity = 'rhou'
    vel = 'velocity'
    advected_interp_method = 'average'
    velocity_interp_method = 'rc'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    ghost_layers = 2
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
  []
  [u_pressure]
    type = FVMomPressure
    variable = u
    momentum_component = 'x'
    vel = 'velocity'
    advected_interp_method = 'average'
  []
  [u_pressure_rz]
    type = FVMomPressureRZ
    variable = u
    p = pressure
  []
  [u_forcing]
    type = FVBodyForce
    variable = u
    function = forcing_u
  []

  [v_advection]
    type = NSFVKernel
    variable = v
    advected_quantity = 'rhov'
    vel = 'velocity'
    advected_interp_method = 'average'
    velocity_interp_method = 'rc'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    ghost_layers = 2
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = ${mu}
  []
  [v_pressure]
    type = FVMomPressure
    variable = v
    momentum_component = 'y'
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
  [diri_u]
    type = FVFunctionDirichletBC
    variable = u
    function = 'exact_u'
    boundary = 'left right top bottom'
  []

  [diri_v]
    type = FVFunctionDirichletBC
    variable = v
    function = 'exact_v'
    boundary = 'left right top bottom'
  []

  [diri_pressure]
    type = FVFunctionDirichletBC
    variable = pressure
    function = 'exact_p'
    boundary = 'left right top bottom'
  []
[]

[Materials]
  [rho]
    type = ADGenericConstantMaterial
    prop_names = 'rho'
    prop_values = ${rho}
  []
  [ins_fv]
    type = INSFVMaterial
    u = u
    v = v
    pressure = 'pressure'
  []
[]

[Functions]
[exact_u]
  type = ParsedFunction
  value = 'sin(x)'
[]
[exact_rhou]
  type = ParsedFunction
  value = 'rho*sin(x)'
  vars = 'rho'
  vals = '${rho}'
[]
[forcing_u]
  type = ParsedFunction
  value = '-rho*sin(x)*sin(y) + cos(x)*cos(y) - (-x*mu*sin(x) + mu*cos(x))/x + (2*x*rho*sin(x)*cos(x) + rho*sin(x)^2)/x'
  vars = 'mu rho'
  vals = '${mu} ${rho}'
[]
[exact_v]
  type = ParsedFunction
  value = 'cos(y)'
[]
[exact_rhov]
  type = ParsedFunction
  value = 'rho*cos(y)'
  vars = 'rho'
  vals = '${rho}'
[]
[forcing_v]
  type = ParsedFunction
  value = 'mu*cos(y) - 2*rho*sin(y)*cos(y) - sin(x)*sin(y) + (x*rho*cos(x)*cos(y) + rho*sin(x)*cos(y))/x'
  vars = 'mu rho'
  vals = '${mu} ${rho}'
[]
[exact_p]
  type = ParsedFunction
  value = 'sin(x)*cos(y)'
[]
[forcing_p]
  type = ParsedFunction
  value = '-sin(y) + (x*cos(x) + sin(x))/x'
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
    type = ElementL2Error
    variable = v
    function = exact_v
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
