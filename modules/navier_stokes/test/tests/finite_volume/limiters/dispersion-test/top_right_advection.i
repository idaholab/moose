[GlobalParams]
  advected_interp_method = 'min_mod' #average upwind sou min_mod vanLeer quick venkatakrishnan skewness-corrected
  velocity_interp_method = 'average'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    a_u = vel_x
    a_v = vel_y
    pressure = pressure
    velocity_interp_method = 'rc'
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 10
    ny = 10
  []
[]

[AuxVariables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = -1.0
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = -1.0
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 1.0
  []
[]

[Variables]
  [scalar]
    type = INSFVScalarFieldVariable
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  [scalar_advection]
    type = INSFVScalarFieldAdvection
    variable = scalar
    rhie_chow_user_object = 'rc'
  []
[]

[FVBCs]
  [fv_inflow]
    type = NSFVOutflowTemperatureBC
    u = vel_x
    v = vel_y
    backflow_T = 1.0
    rho = 1.0
    cp = 1.0
    variable = scalar
    boundary = 'right'
  []
  [fv_outflow]
    type = NSFVOutflowTemperatureBC
    u = vel_x
    v = vel_y
    backflow_T = 0.0
    rho = 1.0
    cp = 1.0
    variable = scalar
    boundary = 'left top bottom'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  dt = 0.1
  end_time = 5.0
  steady_state_detection = false
  steady_state_tolerance = 1e-12
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]
