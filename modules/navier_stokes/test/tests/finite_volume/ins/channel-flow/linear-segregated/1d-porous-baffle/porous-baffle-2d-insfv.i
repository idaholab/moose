mu = 1e-2
rho = 2.0
advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
  advected_interp_method = ${advected_interp_method}
  velocity_interp_method = ${velocity_interp_method}
[]

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5 0.5'
    dy = '0.5'
    ix = '35 35 35'
    iy = '35'
    subdomain_id = '1 2 3'
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'baffle'
  []
  [baffle2]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle
    primary_block = '2'
    paired_block = '3'
    new_boundary = 'baffle2'
  []
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = superficial_u
    v = superficial_v
    pressure = pressure
    porosity = porosity
  []
[]

[Variables]
  [superficial_u]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 0.1
  []
  [superficial_v]
    type = PINSFVSuperficialVelocityVariable
    initial_condition = 0.0
  []
  [pressure]
    type = BernoulliPressureVariable
    initial_condition = 0.0
    porosity = porosity
    rho = ${rho}
    u = superficial_u
    allow_two_term_expansion_on_bernoulli_faces = false
  []
[]

[FVKernels]
  [mass]
    type = PINSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []
  [u_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_u
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_u
    mu = ${mu}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_u
    momentum_component = 'x'
    pressure = pressure
    porosity = porosity
  []
  [u_friction]
    type = PINSFVMomentumFriction
    variable = superficial_u
    Forchheimer_name = forch
    momentum_component = 'x'
    rho = ${rho}
    porosity = porosity
    speed = speed
    standard_friction_formulation = true
    block = 2
  []

  [v_advection]
    type = PINSFVMomentumAdvection
    variable = superficial_v
    rho = ${rho}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_viscosity]
    type = PINSFVMomentumDiffusion
    variable = superficial_v
    mu = ${mu}
    porosity = porosity
    momentum_component = 'y'
  []
  [v_pressure]
    type = PINSFVMomentumPressure
    variable = superficial_v
    momentum_component = 'y'
    pressure = pressure
    porosity = porosity
  []
  [v_friction]
    type = PINSFVMomentumFriction
    variable = superficial_v
    Forchheimer_name = forch
    momentum_component = 'y'
    rho = ${rho}
    porosity = porosity
    speed = speed
    standard_friction_formulation = true
    block = 2
  []
[]

[FVBCs]
  [left_u]
    type = INSFVInletVelocityBC
    boundary = left
    variable = superficial_u
    functor = 0.1
  []
  [top_u]
    type = INSFVInletVelocityBC
    boundary = top
    variable = superficial_u
    functor = 0.0
  []
  [bottom_u]
    type = INSFVInletVelocityBC
    boundary = bottom
    variable = superficial_u
    functor = 0.0
  []

  [left_v]
    type = INSFVInletVelocityBC
    boundary = left
    variable = superficial_v
    functor = 0.0
  []
  [top_v]
    type = INSFVInletVelocityBC
    boundary = top
    variable = superficial_v
    functor = 0.0
  []
  [bottom_v]
    type = INSFVInletVelocityBC
    boundary = bottom
    variable = superficial_v
    functor = 0.0
  []

  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = right
    variable = pressure
    function = 0.0
  []
[]

[FunctorMaterials]
  [forch]
    type = ADGenericVectorFunctorMaterial
    prop_names = forch
    prop_values = '1000 1000 1000'
  []
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 0.5 2 1.0 3 0.5'
  []
  [speed_material]
    type = PINSFVSpeedFunctorMaterial
    porosity = porosity
    superficial_vel_x = superficial_u
    superficial_vel_y = superficial_v
  []
[]

[Postprocessors]
  [p_left]
    type = SideAverageValue
    variable = pressure
    boundary = left
  []
  [p_right]
    type = SideAverageValue
    variable = pressure
    boundary = right
  []
  [p_jump]
    type = ParsedPostprocessor
    expression = 'p_left - p_right'
    pp_names = 'p_left p_right'
  []
  [p_block_1]
    type = ElementAverageValue
    variable = pressure
    block = 1
  []
  [p_block_2]
    type = ElementAverageValue
    variable = pressure
    block = 2
  []
  [p_block_jump]
    type = ParsedPostprocessor
    expression = 'p_block_1 - p_block_2'
    pp_names = 'p_block_1 p_block_2'
  []
  [u_block_1]
    type = ElementAverageValue
    variable = superficial_u
    block = 1
  []
  [u_block_2]
    type = ElementAverageValue
    variable = superficial_u
    block = 2
  []
  [u_block_jump]
    type = ParsedPostprocessor
    expression = 'u_block_1 - u_block_2'
    pp_names = 'u_block_1 u_block_2'
  []
[]

[VectorPostprocessors]
  [u_line]
    type = LineValueSampler
    variable = superficial_u
    start_point = '0 0.5 0'
    end_point = '1.5 0.5 0'
    num_points = 61
    sort_by = id
  []
  [p_line]
    type = LineValueSampler
    variable = pressure
    start_point = '0 0.5 0'
    end_point = '1.5 0.5 0'
    num_points = 61
    sort_by = id
  []
[]

[AuxVariables]
  [porosity_aux]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [por]
    type = FunctorAux
    variable = porosity_aux
    functor = porosity
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  l_tol = 1e-10
  # print_fields = true
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
[]

[Outputs]
  exodus = true
  csv = true
[]
