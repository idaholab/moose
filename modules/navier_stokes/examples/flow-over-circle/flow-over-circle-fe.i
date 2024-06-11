[Mesh]
  second_order = true
[]

[Variables]
  [vel]
    order = SECOND
    family = LAGRANGE_VEC
  []
  [p][]
[]

[Kernels]
  [mass]
    type = INSADConservativeMass
    variable = p
    velocity = vel
  []
  [momentum_time]
    type = INSADMomentumTimeDerivative
    variable = vel
  []
  [momentum_convection]
    type = INSADMomentumConservativeAdvection
    variable = vel
    rho = rho
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
    integrate_p_by_parts = true
  []
[]

[BCs]
  [inlet_mass]
    type = INSADConservativeMassWeakDiriBC
    variable = p
    velocity = 'inlet'
    boundary = 'left_boundary'
  []
  [inlet_momentum_advection]
    type = INSADMomentumConservativeAdvectionWeakDiriBC
    variable = vel
    rho = 'rho'
    velocity = 'inlet'
    boundary = 'left_boundary'
  []
  [inlet_momentum_stress]
    type = INSADMomentumZeroViscousStreeBC
    variable = vel
    boundary = 'left_boundary'
    pressure = p
    integrate_p_by_parts = true
  []
  [circle]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'circle'
    preset = true
  []
  [walls]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'top_boundary bottom_boundary'
    preset = true
  []
  [outlet_mass]
    type = INSADConservativeMassImplicitBC
    variable = p
    velocity = vel
    boundary = 'right_boundary'
  []
  [outlet_momentum]
    type = INSADMomentumConservativeAdvectionImplicitBC
    variable = vel
    rho = 'rho'
    boundary = 'right_boundary'
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho}  ${mu}'
  []
  [mat]
    type = INSADMaterial
    velocity = vel
    pressure = p
  []
[]

[Functions]
  [inlet]
    type = ParsedVectorFunction
    expression_x = ${inlet_velocity}
  []
[]

[Preconditioning]
  [FSP]
    type = FSP
    topsplit = 'by_diri_others'
    [by_diri_others]
      splitting = 'diri others'
      splitting_type  = additive
      petsc_options_iname = '-ksp_type'
      petsc_options_value = 'preonly'
    []
      [diri]
        sides = 'top_boundary bottom_boundary circle'
        vars = 'vel'
        petsc_options_iname = '-ksp_type'
        petsc_options_value = 'none'
      []
      [others]
        splitting = 'u p'
        unside_by_var_boundary_name = 'top_boundary bottom_boundary circle'
        unside_by_var_var_name = 'vel vel vel'
      []
        [u]
          vars = 'vel'
          unside_by_var_boundary_name = 'top_boundary bottom_boundary circle'
          unside_by_var_var_name = 'vel vel vel'
        []
        [p]
          vars = 'p'
        []
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  line_search = 'none'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 10
  end_time = 15
  dtmax = 2e-2
  dtmin = 1e-5
  scheme = 'bdf2'
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-3
    optimal_iterations = 6
    growth_factor = 1.5
  []
[]

[Outputs]
  exodus = true
  csv = true
  checkpoint = true
  print_linear_residuals = false
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    expression = 'rho * U * D / mu'
    constant_names = 'rho U D mu'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius} ${mu}'
  []
  [drag_force]
    type = IntegralDirectedSurfaceForceFE
    velocity = vel
    mu = ${mu}
    pressure = p
    principal_direction = '1 0 0'
    boundary = 'circle'
    outputs = none
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [drag_coeff]
    type = ParsedPostprocessor
    expression = '2*drag_force/rho/(avgvel*avgvel)/D'
    constant_names = 'rho avgvel D'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius}'
    pp_names = 'drag_force'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lift_force]
    type = IntegralDirectedSurfaceForceFE
    velocity = vel
    mu = ${mu}
    pressure = p
    principal_direction = '0 -1 0'
    boundary = 'circle'
    outputs = none
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [lift_coeff]
    type = ParsedPostprocessor
    expression = '2*lift_force/rho/(avgvel*avgvel)/D'
    constant_names = 'rho avgvel D'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius}'
    pp_names = 'lift_force'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
