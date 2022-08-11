mu=4e-3
rho=1
velocity_interp_method = 'rc'
advected_interp_method = 'average'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  # file = first_order_flow_refined.msh
  file = sphere_hybrid.e
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    w = vel_z
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    two_term_boundary_expansion = false
  []
  [vel_z]
    type = INSFVVelocityVariable
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  # mass
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  [w_advection]
    type = INSFVMomentumAdvection
    variable = vel_z
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'z'
  []
  [w_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_z
    mu = ${mu}
    momentum_component = 'z'
  []
  [w_pressure]
    type = INSFVMomentumPressure
    variable = vel_z
    momentum_component = 'z'
    pressure = pressure
  []
[]

[FVBCs]
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'no_slip inlet'
    function = 0
  []

  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'no_slip inlet'
    function = 0
  []

  [no_slip_z]
    type = INSFVNoSlipWallBC
    variable = vel_z
    boundary = 'no_slip'
    function = 0
  []

  [inlet_z]
    type = INSFVInletVelocityBC
    variable = vel_z
    boundary = 'inlet'
    function = inlet_func
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'outlet'
    function = 0
  []
[]

[Functions]
  [./inlet_func]
    type = ParsedFunction
    value = 'sqrt((x-2)^2 * (x+2)^2 * (y-2)^2 * (y+2)^2) / 16'
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho}  ${mu}'
  [../]
[]

[Preconditioning]
  [./SMP] #What is PJFNK
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Transient
  dt = .5
  dtmin = 5e-4
  num_steps = 5
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-12
  nl_max_its = 10
  l_tol = 1e-6
  l_max_its = 250
[]

[Outputs]
    execute_on = 'timestep_end initial'
    print_perf_log = true
    exodus = true
    csv = true
[]
[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    function = 'rho * U * D / mu'
    constant_names = 'rho U D mu'
    constant_expressions = '${rho} 1 2 ${mu}'
    pp_names = ''
  []
[]
