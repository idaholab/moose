mu = 0.01
rho = 2000
u_inlet = 1
advected_interp_method = 'upwind'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 1
    nx = 10
    ny = 6
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.5
  []

  [speed_output]
    type = MooseVariableFVReal
  []

  [vel_x_output]
    type = MooseVariableFVReal
  []

  [vel_y_output]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [speed]
    type = ADFunctorElementalAux
    variable = 'speed_output'
    functor = 'speed'
  []

  [vel_x]
    type = ADFunctorVectorElementalAux
    variable = 'vel_x_output'
    functor = 'velocity'
    component = 0
  []

  [vel_y]
    type = ADFunctorVectorElementalAux
    variable = 'vel_y_output'
    functor = 'velocity'
    component = 1
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = true

    density = ${rho}
    dynamic_viscosity = ${mu}
    porosity = 'porosity'

    initial_velocity = '${u_inlet} 1e-6 0'
    initial_pressure = 0.0

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${u_inlet} 0'
    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip symmetry'
    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '0.1'

    momentum_advection_interpolation = ${advected_interp_method}
    mass_advection_interpolation = ${advected_interp_method}
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-11
[]

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [inlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'left'
  []
  [outlet-u]
    type = SideAverageValue
    variable = superficial_vel_x
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
  csv = false
[]
