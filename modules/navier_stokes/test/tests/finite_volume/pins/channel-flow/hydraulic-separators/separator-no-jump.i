# This test describes a test where three parallel channels are
# separated using flow separators that act as slip boundary conditions.
# The different channels have different friction factors
# meaning that we expect different pressure drops.
# Channel 1 expected drop (analytic, Forchheimer only): 5.50E-03 Pa
# Channel 2 expected drop (analytic, Forchheimer only): 4.40E-02 Pa
# Channel 3 expected drop (analytic, Forchheimer only): 1.49E-01 Pa

rho=1.1
mu=1.1
advected_interp_method='average'
velocity_interp_method='rc'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1'
    dy = '0.25 0.25 0.25'
    ix = '5'
    iy = '2 2 2'
    subdomain_id = '1 2 3'
  []
  [separator-1]
    type = SideSetsBetweenSubdomainsGenerator
    new_boundary = 'separator-1'
    primary_block = 1
    paired_block = 2
    input = mesh
  []
  [separator-2]
    type = SideSetsBetweenSubdomainsGenerator
    new_boundary = 'separator-2'
    primary_block = 2
    paired_block = 3
    input = separator-1
  []
  [inlet-1]
    type = ParsedGenerateSideset
    input = separator-2
    combinatorial_geometry = 'y < 0.25 & x < 0.00001'
    replace = true
    new_sideset_name = inlet-1
  []
  [inlet-2]
    type = ParsedGenerateSideset
    input = inlet-1
    combinatorial_geometry = 'y > 0.25 & y < 0.5 & x < 0.00001'
    replace = true
    new_sideset_name = inlet-2
  []
  [inlet-3]
    type = ParsedGenerateSideset
    input = inlet-2
    combinatorial_geometry = 'y > 0.5 & x < 0.00001'
    replace = true
    new_sideset_name = inlet-3
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
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
  []
  [pressure]
    type = BernoulliPressureVariable
    u = u
    v = v
    rho = ${rho}
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
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_x
    momentum_component = 'x'
    mu = ${mu}
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
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_y
    momentum_component = 'y'
    mu = ${mu}
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
  [inlet-u-3]
    type = INSFVInletVelocityBC
    boundary = 'inlet-3'
    variable = superficial_vel_x
    function = '0.3'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'inlet-1 inlet-2 inlet-3'
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
    boundary = 'separator-1 separator-2'
    variable = superficial_vel_x
    momentum_component = 'x'
  []

  [separator-v]
    type = INSFVVelocityHydraulicSeparatorBC
    boundary = 'separator-1 separator-2'
    variable = superficial_vel_y
    momentum_component = 'y'
  []

  [separator-p]
    type = INSFVScalarFieldSeparatorBC
    boundary = 'separator-1 separator-2'
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
    prop_values = '1.0 1.0 1.0'
    block = 1
  []
  [darcy-2]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '2.0 2.0 2.0'
    block = 2
  []
  [darcy-3]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '3.0 3.0 3.0'
    block = 3
  []
  [speed]
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
  [inlet_p3]
    type = SideAverageValue
    variable = 'pressure'
    boundary = 'inlet-3'
  []
  [drop-1]
    type = ParsedPostprocessor
    expression = 'inlet_p1 - outlet'
    pp_names = 'inlet_p1'
    constant_names = 'outlet'
    constant_expressions = '0.4'
  []
  [drop-2]
    type = ParsedPostprocessor
    expression = 'inlet_p2 - outlet'
    pp_names = 'inlet_p2'
    constant_names = 'outlet'
    constant_expressions = '0.4'
  []
  [drop-3]
    type = ParsedPostprocessor
    expression = 'inlet_p3 - outlet'
    pp_names = 'inlet_p3'
    constant_names = 'outlet'
    constant_expressions = '0.4'
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]
