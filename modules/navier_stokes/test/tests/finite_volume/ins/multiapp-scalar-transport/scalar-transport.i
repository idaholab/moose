diff=1e-3
advected_interp_method='average'
velocity_interp_method='rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  active = 'rc'
  [rc]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
    a_u = ax
    a_v = ay
  []

  [rc_bad]
    type = INSFVRhieChowInterpolator
    u = u
    v = v
    pressure = pressure
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 100
    ny = 20
  []
[]

[Variables]
  [scalar]
    type = INSFVScalarFieldVariable
  []
[]

[AuxVariables]
  [ax]
    type = MooseVariableFVReal
  []
  [ay]
    type = MooseVariableFVReal
  []
  [u]
    type = INSFVVelocityVariable
  []
  [v]
    type = INSFVVelocityVariable
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
  [scalar_advection]
    type = INSFVScalarFieldAdvection
    variable = scalar
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
  []
  [scalar_diffusion]
    type = FVDiffusion
    coeff = ${diff}
    variable = scalar
  []
  [scalar_src]
    type = FVBodyForce
    variable = scalar
    value = 0.1
  []
[]

[FVBCs]
  [inlet_scalar]
    type = FVDirichletBC
    boundary = 'left'
    variable = scalar
    value = 1
  []
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = u
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
