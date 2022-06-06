mu = 1
rho = 1

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 0.5 1'
    dy = '0.5 0.5'
    ix = '8 5 8'
    iy = '8 8'
    subdomain_id = '0 1 2
                    1 2 1'
  []

  [top_outlet]
    type = ParsedGenerateSideset
    input = cmg
    combinatorial_geometry = 'x>2.499 & y>0.4999'
    new_sideset_name = top_right
  []

  [bottom_outlet]
    type = ParsedGenerateSideset
    input = top_outlet
    combinatorial_geometry = 'x>2.499 & y<0.50001'
    new_sideset_name = bottom_right
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = 'upwind'
  velocity_interp_method = 'rc'
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
    initial_condition = 1e-6
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
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

  [diodes_against_flow_x]
    type = INSFVFrictionFlowDiode
    resistance = 100
    variable = vel_x
    direction = '-1 0 0'
    block = 1
    momentum_component = 'x'
  []
  [diodes_against_flow_y]
    type = INSFVFrictionFlowDiode
    resistance = 100
    variable = vel_y
    direction = '-1 0 0'
    block = 1
    momentum_component = 'y'
  []

  [diode_free_flow_x]
    type = INSFVFrictionFlowDiode
    resistance = 100
    variable = vel_x
    direction = '1 0 0'
    block = 2
    momentum_component = 'x'
  []
  [diode_free_flow_y]
    type = INSFVFrictionFlowDiode
    resistance = 100
    variable = vel_y
    direction = '1 0 0'
    block = 2
    momentum_component = 'y'
  []
[]

[FVBCs]
  [walls_u]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top bottom'
    function = 0
  []
  [walls_v]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'top bottom'
    function = 0
  []

  [inlet_u]
    type = INSFVInletVelocityBC
    variable = vel_x
    boundary = 'left'
    function = 1
  []
  [inlet_v]
    type = INSFVInletVelocityBC
    variable = vel_y
    boundary = 'left'
    function = 0
  []

  [outlet]
    type = INSFVOutletPressureBC
    variable = pressure
    boundary = 'right'
    function = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'lu       NONZERO               200'
  line_search = 'none'

  nl_abs_tol = 1e-14
[]

[Postprocessors]
  [mdot_top]
    type = VolumetricFlowRate
    boundary = 'top_right'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = ${rho}
  []
  [mdot_bottom]
    type = VolumetricFlowRate
    boundary = 'bottom_right'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = ${rho}
  []
[]

[Outputs]
  exodus = true
[]
