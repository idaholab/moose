mu = 1
rho = 1
advected_interp_method = 'average'
velocity_interp_method = 'rc'

[GlobalParams]
  two_term_boundary_expansion = true
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Mesh]
  type = GeneratedMesh
  nx = 4
  ny = 4
  xmax = 3.9
  ymax = 4.1
  elem_type = TRI3
  dim = 2
[]

[Problem]
  coord_type = 'RZ'
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
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
    # we can think of the axis as a slip wall boundary, no normal velocity and no viscous shear
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
[]

[FVBCs]
  active = 'inlet-u inlet-v free-slip-wall-u free-slip-wall-v outlet-p axis-u axis-v'

  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'bottom'
    variable = vel_x
    function = 0
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'bottom'
    variable = vel_y
    function = 1
  []
  [free-slip-wall-u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'right'
    variable = vel_x
    momentum_component = 'x'
  []
  [free-slip-wall-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'right'
    variable = vel_y
    momentum_component = 'y'
  []
  [no-slip-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'right'
    variable = vel_x
    function = 0
  []
  [no-slip-wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'right'
    variable = vel_y
    function = 0
  []
  [outlet-p]
    type = INSFVOutletPressureBC
    boundary = 'top'
    variable = pressure
    function = 0
  []
  [axis-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'left'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = x
  []
  [axis-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'left'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = y
  []
  [axis-p]
    type = INSFVSymmetryPressureBC
    boundary = 'left'
    variable = pressure
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [in]
    type = SideIntegralVariablePostprocessor
    variable = vel_y
    boundary = 'bottom'
  []
  [out]
    type = SideIntegralVariablePostprocessor
    variable = vel_y
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
[]
