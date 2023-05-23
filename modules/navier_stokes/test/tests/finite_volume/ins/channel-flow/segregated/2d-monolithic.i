mu = 2.6
rho = 1.0
advected_interp_method = 'average'
velocity_interp_method = 'rc'

[Mesh]
  [read]
    type = FileMeshGenerator
    file = 2d-segregated_in.e
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 0.5
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 0.0
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 0.2
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    boundaries_to_force = 'left top bottom'
  []

[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = '1.1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v
    function = '0.0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = u
    function = 0.0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = v
    function = 0.0
  []
  [wall_p]
    type = FVFunctionNeumannBC
    boundary = 'left top bottom'
    variable = pressure
    function = 0.0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 1.4
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
[]

[Postprocessors]
  [inlet_p]
    type = SideAverageValue
    variable = 'pressure'
    boundary = 'left'
  []
  [outlet-u]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
  csv = true
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
