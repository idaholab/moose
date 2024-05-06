mu = 1.1
rho = 1.1
advected_interp_method = 'average'
velocity_interp_method = 'rc'

[GlobalParams]
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
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = -1
    ymax = 1
    nx = 50
    ny = 10
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
  inactive = 'u_friction_quad v_friction_quad'
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
  [u_friction_linear]
    type = PINSFVMomentumFriction
    variable = vel_x
    Darcy_name = friction_coefficient
    momentum_component = 'x'
    rho = ${rho}
    standard_friction_formulation = false
  []
  [u_friction_quad]
    type = PINSFVMomentumFriction
    variable = vel_x
    speed = speed
    Forchheimer_name = friction_coefficient
    momentum_component = 'x'
    rho = ${rho}
    standard_friction_formulation = false
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
  [v_friction_linear]
    type = PINSFVMomentumFriction
    variable = vel_y
    Darcy_name = friction_coefficient
    momentum_component = 'y'
    rho = ${rho}
    standard_friction_formulation = false
  []
  [v_friction_quad]
    type = PINSFVMomentumFriction
    variable = vel_y
    speed = speed
    Forchheimer_name = friction_coefficient
    momentum_component = 'y'
    rho = ${rho}
    standard_friction_formulation = false
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_x
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_y
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
[]

[FunctorMaterials]
  inactive = exponential_friction_coefficient
  [friction_coefficient]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'friction_coefficient'
    prop_values = '25 25 25'
  []
  [speed_material]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = vel_x
    superficial_vel_y = vel_y
    porosity = 1
    vel_x = vel_x_mat
    vel_y = vel_y_mat
  []
  [Re_material]
    type = ReynoldsNumberFunctorMaterial
    speed = speed
    characteristic_length = 2
    rho = ${rho}
    mu = ${mu}
  []
  [exponential_coeff]
    type = ExponentialFrictionMaterial
    friction_factor_name = 'exponential_coeff'
    Re = Re
    c1 = 0.25
    c2 = 0.55
  []
  [exponential_friction_coefficient]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'friction_coefficient'
    prop_values = 'exponential_coeff exponential_coeff exponential_coeff'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
