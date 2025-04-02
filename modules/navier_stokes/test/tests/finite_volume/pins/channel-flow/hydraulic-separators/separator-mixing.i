# This test is designed to check for energy conservation
# in separated channels. The three inlet temperatures should be
# preserved at the outlets.

rho=1.1
mu=1e-4
k=2.1
cp=5.5
advected_interp_method='upwind'
velocity_interp_method='rc'

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.25 1.0 0.25'
    dy = '0.25 0.25 0.25'
    ix = '4 20 4'
    iy = '5 5 5'
    subdomain_id = '1 2 5 1 3 5 1 4 5'
  []
  [separator-1]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '2'
    paired_block = '3'
    new_boundary = 'separator-1'
  []
  [separator-2]
    type = SideSetsBetweenSubdomainsGenerator
    input = separator-1
    primary_block = '3'
    paired_block = '4'
    new_boundary = 'separator-2'
  []
  [jump-1]
    type = SideSetsBetweenSubdomainsGenerator
    input = separator-2
    primary_block = '1'
    paired_block = '2'
    new_boundary = jump-1
  []
  [jump-2]
    type = SideSetsBetweenSubdomainsGenerator
    input = jump-1
    primary_block = '1'
    paired_block = '3'
    new_boundary = jump-2
  []
  [jump-3]
    type = SideSetsBetweenSubdomainsGenerator
    input = jump-2
    primary_block = '1'
    paired_block = '4'
    new_boundary = jump-3
  []
  [outlet-1]
    type = SideSetsBetweenSubdomainsGenerator
    input = jump-3
    primary_block = '2'
    paired_block = '5'
    new_boundary = outlet-1
  []
  [outlet-2]
    type = SideSetsBetweenSubdomainsGenerator
    input = outlet-1
    primary_block = '3'
    paired_block = '5'
    new_boundary = outlet-2
  []
  [outlet-3]
    type = SideSetsBetweenSubdomainsGenerator
    input = outlet-2
    primary_block = '4'
    paired_block = '5'
    new_boundary = outlet-3
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
    u = superficial_vel_x
    v = superficial_vel_y
    rho = ${rho}
    pressure_drop_sidesets = 'jump-1 jump-2 jump-3 outlet-1 outlet-2 outlet-3'
    pressure_drop_form_factors = '0.1 0.2 0.3 0.1 0.2 0.3'
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = 300
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

  [temp_conduction]
    type = FVDiffusion
    coeff = ${k}
    variable = T_fluid
  []
  [temp_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
  []
  [temp_source]
    type = FVBodyForce
    variable = T_fluid
    function = heating
    block = '2 3 4'
  []
[]

[Functions]
  [heating]
    type = ParsedFunction
    expression = 'if(y<0.25, 10, if(y<0.5, 20, 30))'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = superficial_vel_x
    function = '0.1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = superficial_vel_y
    function = 0
  []

  [inlet-T]
    type = FVDirichletBC
    variable = T_fluid
    boundary = 'left'
    value = 300
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
    type = INSFVPressureHydraulicSeparatorBC
    boundary = 'separator-1 separator-2'
    variable = pressure
  []

  [separator-T]
    type = INSFVEnergyHydraulicSeparatorBC
    boundary = 'separator-1 separator-2'
    variable = T_fluid
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 0.4
  []
[]

[FunctorMaterials]
  [porosity]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 0.8
                               2 0.7
                               3 0.6
                               4 0.5
                               5 0.8'
  []
  [darcy-1]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '1.0 1.0 1.0'
    block = '1 5'
  []
  [darcy-2]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '3.0 3.0 3.0'
    block = 2
  []
  [darcy-3]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '1.5 1.5 1.5'
    block = 3
  []
  [darcy-4]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Forchheimer_coefficient'
    prop_values = '0.75 0.75 0.75'
    block = 4
  []
  [speed]
    type = PINSFVSpeedFunctorMaterial
    superficial_vel_x = superficial_vel_x
    superficial_vel_y = superficial_vel_y
    porosity = porosity
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    temperature = 'T_fluid'
    rho = ${rho}
    cp = ${cp}
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'
  line_search = 'none'
  nl_rel_tol = 1e-10
[]

[Postprocessors]
  [outlet_T1]
    type = SideAverageValue
    variable = 'T_fluid'
    boundary = 'right'
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]
