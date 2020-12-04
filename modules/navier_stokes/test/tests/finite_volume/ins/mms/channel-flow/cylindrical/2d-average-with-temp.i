mu=1.1
rho=1.1
k=1.1
cp=1.1
advected_interp_method='average'
velocity_interp_method='average'
force_boundary_execution=true
velocity='velocity'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  []
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = true
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
  [temperature]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    velocity_interp_method = ${velocity_interp_method}
    vel = ${velocity}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
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
    vel = ${velocity}
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
    force_boundary_execution = ${force_boundary_execution}
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    vel = ${velocity}
    advected_interp_method = ${advected_interp_method}
    force_boundary_execution = ${force_boundary_execution}
  []
  [u_pressure_rz]
    type = INSFVMomentumPressureRZ
    variable = u
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
    vel = ${velocity}
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = ${mu}
    force_boundary_execution = ${force_boundary_execution}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    vel = ${velocity}
    advected_interp_method = ${advected_interp_method}
    force_boundary_execution = ${force_boundary_execution}
  []
  [v_forcing]
    type = FVBodyForce
    variable = v
    function = forcing_v
  []

  [temp_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = temperature
    force_boundary_execution = ${force_boundary_execution}
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    vel = ${velocity}
    variable = temperature
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
  []
  [temp_forcing]
    type = FVBodyForce
    variable = temperature
    function = forcing_t
  []
[]

[FVBCs]
  [axis-inlet-wall-u]
    type = FVFunctionDirichletBC
    boundary = 'left bottom right'
    variable = u
    function = 'exact_u'
  []
  [inlet-v]
    type = FVFunctionDirichletBC
    boundary = 'bottom'
    variable = v
    function = 'exact_v'
  []
  [axis-inlet-wall-t]
    type = FVFunctionDirichletBC
    boundary = 'left bottom right'
    variable = temperature
    function = 'exact_t'
  []
  [outlet_p]
    type = FVFunctionDirichletBC
    boundary = 'top'
    variable = pressure
    function = 'exact_p'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'k cp'
    prop_values = '${k} ${cp}'
  []
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    temperature = 'temperature'
    rho = ${rho}
  []
[]

[Functions]
[exact_u]
  type = ParsedFunction
  value = 'sin(x*pi)*sin((1/2)*y*pi)'
[]
[exact_rhou]
  type = ParsedFunction
  value = 'rho*sin(x*pi)*sin((1/2)*y*pi)'
  vars = 'rho'
  vals = '${rho}'
[]
[forcing_u]
  type = ParsedFunction
  value = '(1/4)*pi^2*mu*sin(x*pi)*sin((1/2)*y*pi) - pi*sin(x*pi)*cos((1/2)*y*pi) - (-x*pi^2*mu*sin(x*pi)*sin((1/2)*y*pi) + pi*mu*sin((1/2)*y*pi)*cos(x*pi))/x + (2*x*pi*rho*sin(x*pi)*sin((1/2)*y*pi)^2*cos(x*pi) + rho*sin(x*pi)^2*sin((1/2)*y*pi)^2)/x + (-x*pi*rho*sin(x*pi)*sin((1/2)*y*pi)*sin(y*pi)*cos(x*pi) + (1/2)*x*pi*rho*sin(x*pi)*cos(x*pi)*cos((1/2)*y*pi)*cos(y*pi))/x'
  vars = 'mu rho'
  vals = '${mu} ${rho}'
[]
[exact_v]
  type = ParsedFunction
  value = 'cos(x*pi)*cos(y*pi)'
[]
[exact_rhov]
  type = ParsedFunction
  value = 'rho*cos(x*pi)*cos(y*pi)'
  vars = 'rho'
  vals = '${rho}'
[]
[forcing_v]
  type = ParsedFunction
  value = 'pi^2*mu*cos(x*pi)*cos(y*pi) - 2*pi*rho*sin(y*pi)*cos(x*pi)^2*cos(y*pi) - 1/2*pi*sin((1/2)*y*pi)*cos(x*pi) - (-x*pi^2*mu*cos(x*pi)*cos(y*pi) - pi*mu*sin(x*pi)*cos(y*pi))/x + (-x*pi*rho*sin(x*pi)^2*sin((1/2)*y*pi)*cos(y*pi) + x*pi*rho*sin((1/2)*y*pi)*cos(x*pi)^2*cos(y*pi) + rho*sin(x*pi)*sin((1/2)*y*pi)*cos(x*pi)*cos(y*pi))/x'
  vars = 'mu rho'
  vals = '${mu} ${rho}'
[]
[exact_p]
  type = ParsedFunction
  value = 'cos(x*pi)*cos((1/2)*y*pi)'
[]
[forcing_p]
  type = ParsedFunction
  value = '-pi*rho*sin(y*pi)*cos(x*pi) + (x*pi*rho*sin((1/2)*y*pi)*cos(x*pi) + rho*sin(x*pi)*sin((1/2)*y*pi))/x'
  vars = 'rho'
  vals = '${rho}'
[]
[exact_t]
  type = ParsedFunction
  value = 'sin(x*pi)*sin((1/2)*y*pi)'
[]
[forcing_t]
  type = ParsedFunction
  value = '(1/4)*pi^2*k*sin(x*pi)*sin((1/2)*y*pi) - (-x*pi^2*k*sin(x*pi)*sin((1/2)*y*pi) + pi*k*sin((1/2)*y*pi)*cos(x*pi))/x + (2*x*pi*cp*rho*sin(x*pi)*sin((1/2)*y*pi)^2*cos(x*pi) + cp*rho*sin(x*pi)^2*sin((1/2)*y*pi)^2)/x + (-x*pi*cp*rho*sin(x*pi)*sin((1/2)*y*pi)*sin(y*pi)*cos(x*pi) + (1/2)*x*pi*cp*rho*sin(x*pi)*cos(x*pi)*cos((1/2)*y*pi)*cos(y*pi))/x'
  vars = 'k rho cp'
  vals = '${k} ${rho} ${cp}'
[]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
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
  [./L2t]
    variable = temperature
    function = exact_t
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
[]
