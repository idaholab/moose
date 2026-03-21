rho = 1.0
mu = 1e-2 # 1
U = 1.0

advected_interp_method = 'average'
velocity_interp_method = 'rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'channel_expansion_contraction.msh'
  []
  uniform_refine = 0
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = ${U}
    two_term_boundary_expansion = true
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0
    two_term_boundary_expansion = true
  []
  [pressure]
    type = INSFVPressureVariable
    two_term_boundary_expansion = true
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
  [inlet_u]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = vel_x
    functor = '${U}'
  []
  [inlet_v]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = vel_y
    functor = '0'
  []
  [free_slip_u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom'
    variable = vel_x
    momentum_component = 'x'
  []
  [free_slip_v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom'
    variable = vel_y
    momentum_component = 'y'
  []
  # [walls-u]
  #   type = INSFVNoSlipWallBC
  #   boundary = 'top'
  #   variable = vel_x
  #   function = 0
  # []
  # [walls-v]
  #   type = INSFVNoSlipWallBC
  #   boundary = 'top'
  #   variable = vel_y
  #   function = 0
  # []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet'
    variable = pressure
    function = '0'
  []
[]

[FunctorMaterials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  nl_rel_tol = 1e-8
[]

[Postprocessors]
  [inlet_pressure]
    type = SideAverageValue
    variable = pressure
    boundary = 'inlet'
  []
  [outlet_pressure]
    type = SideAverageValue
    variable = pressure
    boundary = 'outlet'
  []
  [pressure_drop]
    type = DifferencePostprocessor
    value1 = inlet_pressure
    value2 = outlet_pressure
  []
  [inlet_vel_x_avg]
    type = SideAverageValue
    variable = vel_x
    boundary = 'inlet'
  []
  [outlet_vel_x_avg]
    type = SideAverageValue
    variable = vel_x
    boundary = 'outlet'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
