mu=1.1
rho=1.1
advected_interp_method='average'
velocity_interp_method='rc'

[GlobalParams]
  two_term_boundary_expansion = true
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
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
    ny = 9
  []
  [subdomain]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '5 -1 0'
    top_right = '10 1 0'
    block_id = 1
    input = gen
  []
[]

[Problem]
  fv_bcs_integrity_check = true
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
  inactive='u_friction_quad v_friction_quad'
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []
  [u_friction_linear]
    type = PINSFVMomentumFriction
    variable = u
    porosity = 1.0
    rho = ${rho}
    Darcy_name = friction_coefficient
    momentum_component = 'x'
    block = '1'
  []
  [u_friction_quad]
    type = PINSFVMomentumFriction
    variable = u
    porosity = 1.0
    rho = ${rho}
    Forchheimer_name = friction_coefficient
    momentum_component = 'x'
    block = '1'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
  []
  [v_friction_linear]
    type = PINSFVMomentumFriction
    variable = v
    porosity = 1.0
    rho = ${rho}
    Darcy_name = friction_coefficient
    momentum_component = 'y'
    block = '1'
  []
  [v_friction_quad]
    type = PINSFVMomentumFriction
    variable = v
    porosity = 1.0
    rho = ${rho}
    Forchheimer_name = friction_coefficient
    momentum_component = 'y'
    block = '1'
  []
[]

[FVBCs]
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
    function = '0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = u
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = v
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
[]

[Materials]
  [friction_coefficient]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'friction_coefficient'
    prop_values = '25 25 25'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      200                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
