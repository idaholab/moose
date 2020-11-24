mu=1.1
rho=1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 2
    xmax = 3
    nx = 2
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
  [pressure]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  [mass]
    type = NSFVMassAdvection
    variable = pressure
    advected_interp_method = 'average'
    velocity_interp_method = 'rc'
    vel = 'velocity'
    pressure = pressure
    u = u
    mu = ${mu}
    rho = ${rho}
  []
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
  []

  [u_advection]
    type = NSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = 'velocity'
    advected_interp_method = 'average'
    velocity_interp_method = 'rc'
    pressure = pressure
    u = u
    mu = ${mu}
    rho = ${rho}
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
  []
  [u_pressure]
    type = NSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    vel = 'velocity'
    advected_interp_method = 'average'
  []
  [u_pressure_rz]
    type = NSFVMomentumPressureRZ
    variable = u
    p = pressure
  []
  [u_forcing]
    type = FVBodyForce
    variable = u
    function = forcing_u
  []
[]

[FVBCs]
  [diri_u]
    type = FVFunctionDirichletBC
    variable = u
    function = 'exact_u'
    boundary = 'left right'
  []

  [diri_pressure]
    type = FVFunctionDirichletBC
    variable = pressure
    function = 'exact_p'
    boundary = 'left right'
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
  value = '-sin(x) - (-x*mu*sin(x) + mu*cos(x))/x + (2*x*rho*sin(x)*cos(x) + rho*sin(x)^2)/x'
  vars = 'mu rho'
  vals = '${mu} ${rho}'
[]
[exact_p]
  type = ParsedFunction
  value = 'cos(x)'
[]
[forcing_p]
  type = ParsedFunction
  value = '(x*rho*cos(x) + rho*sin(x))/x'
  vars = 'rho'
  vals = '${rho}'
[]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
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
  [./L2p]
    variable = pressure
    function = exact_p
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  [../]
[]
