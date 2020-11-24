mu=1
rho=1
advected_interp_method='average'
velocity_interp_method='rc'
force_boundary_execution=false
mass_boundaries_to_force='bottom top'
momentum_boundaries_to_force=''
penalty=1e6

[Mesh]
  file = diverging.msh
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
    initial_condition = 1e-15
  []
  [v]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1e-15
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
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    vel = 'velocity'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
    boundaries_to_force = ${mass_boundaries_to_force}
  []

  [u_advection]
    type = NSFVMomentumAdvection
    variable = u
    advected_quantity = 'rhou'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
    boundaries_to_force = ${momentum_boundaries_to_force}
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
    force_boundary_execution = ${force_boundary_execution}
    boundaries_to_force = ${momentum_boundaries_to_force}
  []
  [u_pressure]
    type = NSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    force_boundary_execution = ${force_boundary_execution}
    boundaries_to_force = ${momentum_boundaries_to_force}
  []
  [u_pressure_rz]
    type = NSFVMomentumPressureRZ
    variable = u
    p = pressure
  []

  [v_advection]
    type = NSFVMomentumAdvection
    variable = v
    advected_quantity = 'rhov'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
    force_boundary_execution = ${force_boundary_execution}
    boundaries_to_force = ${momentum_boundaries_to_force}
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = ${mu}
    force_boundary_execution = ${force_boundary_execution}
    boundaries_to_force = ${momentum_boundaries_to_force}
  []
  [v_pressure]
    type = NSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    vel = 'velocity'
    advected_interp_method = ${advected_interp_method}
    force_boundary_execution = ${force_boundary_execution}
    boundaries_to_force = ${momentum_boundaries_to_force}
  []
[]

[FVBCs]
  active = 'inlet-u inlet-v free-slip-wall-u free-slip-wall-v'

  # Note that how the inlet is handled is not quite right for a no-slip problem. We must operate our
  # mass and momentum flux kernels on the inlet because clearly there is mass and momentum being
  # transported across the boundary. However, as documented in #16169 we implicitly apply a
  # zero-normal-gradient condition for the pressure at the inlet, and this affects the results in
  # the vicinity. The inlet mass flow rate should be equal to pi, however, the result with the
  # current set of bcs is a little higher than that
  [inlet-u]
    type = FVDirichletBC
    boundary = 'bottom'
    variable = u
    value = 0
  []
  [inlet-v]
    type = FVDirichletBC
    boundary = 'bottom'
    variable = v
    value = 1
  []
  [free-slip-wall-u]
    type = NSFVPenaltyFreeSlipBC
    boundary = 'right'
    variable = u
    momentum_component = x
    u = u
    v = v
    penalty = ${penalty}
  []
  [free-slip-wall-v]
    type = NSFVPenaltyFreeSlipBC
    boundary = 'right'
    variable = v
    momentum_component = y
    u = u
    v = v
    penalty = ${penalty}
  []
  [no-slip-wall-u]
    type = FVDirichletBC
    boundary = 'right'
    variable = u
    value = 0
  []
  [no-slip-wall-v]
    type = FVDirichletBC
    boundary = 'right'
    variable = v
    value = 0
  []
  [outlet-p]
    type = FVDirichletBC
    boundary = 'top'
    variable = pressure
    value = 0
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
    u = 'u'
    v = 'v'
    pressure = 'pressure'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options = '-options_left'
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'asm      lu           NONZERO                   200'
  line_search = 'none'
[]

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [in]
    type = SideIntegralVariablePostprocessor
    variable = v
    boundary = 'bottom'
  []
  [out]
    type = SideIntegralVariablePostprocessor
    variable = v
    boundary = 'top'
  []
  [num_lin]
    type = NumLinearIterations
    outputs = 'console'
  []
  [num_nl]
    type = NumNonlinearIterations
    outputs = 'console'
  []
  [cum_lin]
    type = CumulativeValuePostprocessor
    outputs = 'console'
    postprocessor = 'num_lin'
  []
  [cum_nl]
    type = CumulativeValuePostprocessor
    outputs = 'console'
    postprocessor = 'num_nl'
  []
[]

[Outputs]
  exodus = true
  csv = true
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]
