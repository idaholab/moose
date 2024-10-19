diff = 1e-12
advected_interp_method = 'vanLeer' #average upwind sou min_mod vanLeer quick venkatakrishnan skewness-corrected
velocity_interp_method = 'average'

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    a_u = vel_x
    a_v = vel_y
    pressure = pressure
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
    nx = 50
    ny = 50
  []
[]

[AuxVariables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1.0
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1.0
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 1.0
  []
[]

[Variables]
  [scalar]
    type = INSFVScalarFieldVariable
  []
[]

[FVKernels]
  [scalar_time]
    type = FVFunctorTimeKernel
    variable = scalar
  []
  [scalar_advection]
    type = INSFVScalarFieldAdvection
    variable = scalar
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [scalar_diffusion]
    type = FVDiffusion
    coeff = ${diff}
    variable = scalar
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
    boundary = 'left'
  []
  [fv_outflow]
    type = NSFVOutflowTemperatureBC
    u = vel_x
    v = vel_y
    backflow_T = 0.0
    rho = 1.0
    cp = 1.0
    variable = scalar
    boundary = 'right top bottom'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 20
    linear_iteration_ratio = 2
    dt = 0.1
  []
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
  csv = true
[]
