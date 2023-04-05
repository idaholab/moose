[Mesh]
  file = '2d_cone.msh'
  coord_type = RZ
[]

[Variables]
  [vel_x]
    order = SECOND
  []
  [vel_y]
    order = SECOND
  []
  [p][]
[]

[Kernels]
  [momentum_x_time]
    type = TimeDerivative
    variable = vel_x
  []
  [momentum_x_convection]
    type = ADAdvection
    variable = vel_x
    velocity = 'velocity'
  []
  [momentum_x_diffusion]
    type = MatDiffusion
    variable = vel_x
    diffusivity = 1
  []
  [momentum_x_diffusion_rz]
    type = ADMomentumViscousRZ
    variable = vel_x
    mu_name = 1
    component = 0
  []
  [momentum_x_pressure]
    type = PressureGradient
    integrate_p_by_parts = true
    variable = vel_x
    pressure = p
    component = 0
  []
  [momentum_y_time]
    type = TimeDerivative
    variable = vel_y
  []
  [momentum_y_convection]
    type = ADAdvection
    variable = vel_y
    velocity = 'velocity'
  []
  [momentum_y_diffusion]
    type = MatDiffusion
    variable = vel_y
    diffusivity = 1
  []
  [momentum_y_diffusion_rz]
    type = ADMomentumViscousRZ
    variable = vel_y
    mu_name = 1
    component = 1
  []
  [momentum_y_pressure]
    type = PressureGradient
    integrate_p_by_parts = true
    variable = vel_y
    pressure = p
    component = 1
  []
  [mass]
    type = ADMassAdvection
    variable = p
    vel_x = vel_x
    vel_y = vel_y
  []
[]

[BCs]
  [u_in]
    type = DirichletBC
    boundary = bottom
    variable = vel_x
    value = 0
  []
  [v_in]
    type = FunctionDirichletBC
    boundary = bottom
    variable = vel_y
    function = 'inlet_func'
  []
  [u_axis_and_walls]
    type = DirichletBC
    boundary = 'left right'
    variable = vel_x
    value = 0
  []
  [v_no_slip]
    type = DirichletBC
    boundary = 'right'
    variable = vel_y
    value = 0
  []
[]

[Materials]
  [vel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = vel_x
    v = vel_y
  []
[]

[Functions]
  [inlet_func]
    type = ParsedFunction
    expression = '-4 * x^2 + 1'
  []
[]

[Executioner]
  type = Transient
  dt = 0.005
  dtmin = 0.005
  num_steps = 5
  l_max_its = 100

  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = 'bjacobi  ilu          4'

  nl_rel_tol = 1e-12
  nl_max_its = 6
[]

[Outputs]
  exodus = true
[]
