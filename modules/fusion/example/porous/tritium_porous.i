# https://www.engineeringtoolbox.com/gases-absolute-dynamic-viscosity-d_1888.html
mu= 2e-5 # Pa.s
# https://www.engineeringtoolbox.com/helium-density-specific-weight-temperature-pressure-d_2090.html
# 105058 Pa (14.5 psia), 300K
rho = 0.1604 # kg/m^3
advected_interp_method='upwind'
velocity_interp_method='rc'

[Mesh]
  file = tritium_porous.e
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = superficial_vel_x
    v = superficial_vel_y
    w = superficial_vel_z
    pressure = pressure
    porosity = porosity
  []
[]

[Variables]
  [superficial_vel_x]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
  []
  [superficial_vel_y]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
  []
  [superficial_vel_z]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 1e-6
  []
  [pressure]
    type = INSFVPressureVariable
    #scaling = 1e-6
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.1
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

  [u_time]
    type = PINSFVMomentumTimeDerivative
    variable = superficial_vel_x
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_x
    mu = ${mu}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_x
    momentum_component = 'x'
    pressure = pressure
    porosity = porosity
  []
  [u_friction]
    type = INSFVMomentumFriction
    variable = superficial_vel_x
    momentum_component = 'x'
    linear_coef_name = 'Darcy_coefficient'
  []

  [v_time]
    type = PINSFVMomentumTimeDerivative
    variable = superficial_vel_y
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_y
    mu = ${mu}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_y
    momentum_component = 'y'
    pressure = pressure
    porosity = porosity
  []
  [v_friction]
    type = INSFVMomentumFriction
    variable = superficial_vel_y
    momentum_component = 'y'
    linear_coef_name = 'Darcy_coefficient'
  []

  [w_time]
    type = PINSFVMomentumTimeDerivative
    variable = superficial_vel_z
    rho = ${rho}
    momentum_component = 'z'
  []
  [w_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_vel_z
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'z'
  []
  [w_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_vel_z
    mu = ${mu}
    porosity = porosity
    momentum_component = 'z'
  []
  [w_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_vel_z
    momentum_component = 'z'
    pressure = pressure
    porosity = porosity
  []
  [w_friction]
    type = INSFVMomentumFriction
    variable = superficial_vel_z
    momentum_component = 'z'
    linear_coef_name = 'Darcy_coefficient'
  []
[]

[FVBCs]
  # Inlet mass for the entire blanket sector
  # The blanket is 132 larger than a TBM.
  # TBM: 0.5 * 0.5 * 1.7
  # FNSF blanket: 2.8 * 2.8 * 7.2
  # 800 Nm^3 / h
  # 0.044 * 4 * 800 = 141 kg/h = 0.04 kg/s
  # The total inlet area (max) : 1.883
  # mfr / (area * rho)
  # Averaged velocity: 0.13
  [inlet-u]
    type = INSFVInletVelocityNormalBC
    boundary = 'bottom'
    variable = superficial_vel_x
    direction = x
    function = '0.13'
  []
  [inlet-v]
    type = INSFVInletVelocityNormalBC
    boundary = 'bottom'
    variable = superficial_vel_y
    direction = y
    function = '0.13'
  []
  [inlet-w]
    type = INSFVInletVelocityNormalBC
    boundary = 'bottom'
    variable = superficial_vel_z
    direction = z
    # 20 cm/s
    function = '0.13'
  []

  [no-slip-u]
    type = INSFVNoSlipWallBC
    boundary = 'outer inner left right'
    variable = superficial_vel_x
    function = 0
  []
  [no-slip-v]
    type = INSFVNoSlipWallBC
    boundary = 'outer inner left right'
    variable = superficial_vel_y
    function = 0
  []
  [no-slip-w]
    type = INSFVNoSlipWallBC
    boundary = 'outer inner left right'
    variable = superficial_vel_z
    function = 0
  []

  [outlet-p]
    type = INSFVOutletPressureBC
    boundary = 'top'
    variable = pressure
    # Cellular breeder summary
    # 788 and 790 torr regardless of fill level in the balloon
    function = 105058
    #function = 0
  []
[]

[Materials]
  [darcy]
    type = ADGenericFunctorMaterial
    # Permeability: 6.5 Darcy (1 Darcy = 1e-12 m^2)
    # Darcy: mu / k
    # Need to multiply porosity as well
    # mu: fluid dynamic viscosity
    # k: permeability
    # mu = 2e-5, porosity = 0.1, k = 6.5e-12
    prop_names = 'Darcy_coefficient'
    prop_values = '3e7'
    #prop_values = '0.1'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      200                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-9
  end_time = 1
  steady_state_detection = true
  #automatic_scaling = true
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 8
    dt = 1e-1
    growth_factor = 1.2
  []
[]

# Some basic Postprocessors to visually examine the solution
[Postprocessors]
  [inlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'bottom'
  []
  [outlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'top'
  []
  [outlet-u]
    type = SideIntegralVariablePostprocessor
    variable = superficial_vel_x
    boundary = 'top'
  []
  [inlet-u]
    type = SideIntegralVariablePostprocessor
    variable = superficial_vel_x
    boundary = 'bottom'
  []

  [inlet_mfr]
    type = VolumetricFlowRate
    boundary = bottom
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = ${rho}
  []
  [outlet_mfr]
    type = VolumetricFlowRate
    boundary = top
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    vel_z = superficial_vel_z
    advected_quantity = ${rho}
  []
  [inlet_area]
    type = AreaPostprocessor
    boundary = bottom
  []
[]

[Outputs]
  exodus = true
  [./debug] # This is a test, use the [Debug] block to enable this
    type = VariableResidualNormsDebugOutput
  [../]
[]
