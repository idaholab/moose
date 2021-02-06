mu=1
rho=1
advected_interp_method='average'
velocity_interp_method='rc'

[GlobalParams]
  two_term_boundary_expansion = true
[]

[Mesh]
  file = diverging.msh
  uniform_refine = 2
[]

[Problem]
  fv_bcs_integrity_check = true
  coord_type = 'RZ'
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1e-15
  []
  [v]
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
    vel = 'velocity'
    pressure = pressure
    u = u
    v = v
    mu = ${mu}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
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
  []
  [u_viscosity]
    type = FVDiffusion
    variable = u
    coeff = ${mu}
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    p = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
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
    # we can think of the axis as a slip wall boundary, no normal velocity and no viscous shear
  []
  [v_viscosity]
    type = FVDiffusion
    variable = v
    coeff = ${mu}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    p = pressure
  []
[]

[FVBCs]
  active = 'inlet-u inlet-v free-slip-wall-u free-slip-wall-v outlet-p axis-u axis-v'

  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'bottom'
    variable = u
    function = 0
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'bottom'
    variable = v
    function = 1
  []
  [free-slip-wall-u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'right'
    variable = u
  []
  [free-slip-wall-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'right'
    variable = v
  []
  [no-slip-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'right'
    variable = u
    function = 0
  []
  [no-slip-wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'right'
    variable = v
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
    variable = u
    u = u
    v = v
    mu = ${mu}
    momentum_component = x
  []
  [axis-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'left'
    variable = v
    u = u
    v = v
    mu = ${mu}
    momentum_component = y
  []
  [axis-p]
    type = INSFVSymmetryPressureBC
    boundary = 'left'
    variable = pressure
  []
[]

[Materials]
  [ins_fv]
    type = INSFVMaterial
    u = 'u'
    v = 'v'
    pressure = 'pressure'
    rho = ${rho}
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
[]
