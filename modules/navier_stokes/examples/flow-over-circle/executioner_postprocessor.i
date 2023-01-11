[Functions]
  [inlet_function]
    type = ParsedFunction
    expression = '4*U*(y-ymin)*(ymax-y)/(ymax-ymin)/(ymax-ymin)'
    symbol_names = 'U ymax ymin'
    symbol_values = '${inlet_velocity} ${y_max} ${y_min}'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
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
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    function = 'rho * U * D / mu'
    constant_names = 'rho U D mu'
    constant_expressions = '${rho} ${fparse 2/3*inlet_velocity} ${fparse 2*circle_radius} ${mu}'
    pp_names = ''
  []
  [point_vel_x]
    type = PointValue
    point = '${fparse (x_max-x_min)/2} ${fparse (y_max-y_min)/2} 0'
    variable = 'vel_x'
  []
  [point_vel_y]
    type = PointValue
    point = '${fparse (x_max-x_min)/2} ${fparse (y_max-y_min)/2} 0'
    variable = 'vel_y'
  []
[]
