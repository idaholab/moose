mu = 1.1
rho = 1

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '20'
    dy = '1.0'
    ix = '10'
    iy = '4'
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = true

    density = 'rho'
    dynamic_viscosity = 'mu'
    porosity = 'porosity'

    initial_velocity = '1 0 0'
    initial_pressure = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '1 0'
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'slip slip'
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0'

    friction_types = 'darcy'
    friction_coeffs = 'friction_W'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
  []
[]

[Materials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [friction]
    type = LinearFrictionFactorFunctorMaterial
    porosity = porosity
    functor_name = friction_W
    superficial_vel_x = superficial_vel_x
    superficial_vel_y = superficial_vel_y
    f = '0'
    g = '11'
    A = '1 1 1'
    B = '1 1 1'
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[ICs]
  [porosity_ic]
    type = FunctionIC
    variable = porosity
    function = '1 - x / 40'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-14
[]

[Postprocessors]
  # solution is dp/dx = -11 / (1 - x/40)^3
  # dp = -11 * integral_{0}^{20} (1 - x/40)^3 dx = 660
  #
  [inlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'left'
  []
[]

[Outputs]
  exodus = true
[]
