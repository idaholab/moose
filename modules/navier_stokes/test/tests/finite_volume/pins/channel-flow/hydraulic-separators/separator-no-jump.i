rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1'
    dy = '0.25 0.25'
    ix = '2'
    iy = '1 1'
    subdomain_id = '1 2'
  []
  [separator]
    type = SideSetsBetweenSubdomainsGenerator
    new_boundary = 'separator'
    primary_block = 1
    paired_block = 2
    input = mesh
  []
  [inlet-1]
    type = ParsedGenerateSideset
    input = separator
    combinatorial_geometry = 'y < 0.25 & x < 0.00001'
    replace = true
    new_sideset_name = inlet-1
  []
  [inlet-2]
    type = ParsedGenerateSideset
    input = inlet-1
    combinatorial_geometry = 'y > 0.25 & x < 0.00001'
    replace = true
    new_sideset_name = inlet-2
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
  porosity = porosity
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = superficial_vel_x
    v = superficial_vel_y
    pressure = pressure
  []
[]

[Variables]
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 0.1
    two_term_boundary_expansion = false
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 0.0
    two_term_boundary_expansion = false
  []
  [pressure]
    type = BernoulliPressureVariable
    u = u
    v = v
    rho = ${rho}
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  [mass]
    type = PINSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [u_friction]
    type = PINSFVMomentumFriction
    variable = superficial_vel_x
    momentum_component = 'x'
    Forchheimer_name = 'Forchheimer_coefficient'
    rho = ${rho}
    speed = speed
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_y
    pressure = pressure
    momentum_component = 'y'
  []
  [v_friction]
    type = PINSFVMomentumFriction
    variable = superficial_vel_y
    momentum_component = 'y'
    Forchheimer_name = 'Forchheimer_coefficient'
    rho = ${rho}
    speed = speed
  []
[]

[FVBCs]
  [inlet-u-1]
    type = INSFVInletVelocityBC
    boundary = 'inlet-1'
    variable = superficial_vel_x
    function = '0.1'
  []
  [inlet-u-2]
    type = INSFVInletVelocityBC
    boundary = 'inlet-2'
    variable = superficial_vel_x
    function = '0.2'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'inlet-1 inlet-2'
    variable = superficial_vel_y
    function = 0
  []

  [walls-u]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom'
    variable = superficial_vel_x
    momentum_component = 'x'
  []
  [walls-v]
    type = INSFVNaturalFreeSlipBC
    boundary = 'top bottom'
    variable = superficial_vel_y
    momentum_component = 'y'
  []

  [separator-u]
    type = INSFVVelocityHydraulicSeparatorBC
    boundary = 'separator'
    variable = superficial_vel_x
    momentum_component = 'x'
  []

  [separator-v]
    type = INSFVVelocityHydraulicSeparatorBC
    boundary = 'separator'
    variable = superficial_vel_y
    momentum_component = 'y'
  []

  [separator-p]
    type = INSFVPressureHydraulicSeparatorBC
    boundary = 'separator'
    variable = pressure
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0.4
  []
[]

[FunctorMaterials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = '1.0'
  []
  [darcy-1]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '0.1 0.1 0.1'
    block = 1
  []
  [darcy-2]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '0.2 0.2 0.2'
    block = 2
  []
  [speec]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = superficial_vel_x
    superficial_vel_y = superficial_vel_y
    porosity = porosity
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_max_its = 10
[]

[Postprocessors]
  [inlet_p1]
    type = SideAverageValue
    variable = 'pressure'
    boundary = 'inlet-1'
  []
  [inlet_p2]
    type = SideAverageValue
    variable = 'pressure'
    boundary = 'inlet-2'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
