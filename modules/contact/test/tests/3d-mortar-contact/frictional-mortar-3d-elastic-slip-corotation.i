compression = 2e-3
elastic_slip = 2e-3
tangential_shift = 2e-4
rotation_angle = 1.5707963267948966

!include frictional-mortar-3d-action.i

[Mesh]
  [top_block]
    nx := 2
    ny := 2
    nz := 1
    zmin := 0
    zmax := 0.25
  []
  [bottom_block]
    nx := 3
    ny := 3
    nz := 1
    zmin := -0.25
    zmax := 0
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    incremental = true
  []
[]

[Contact]
  [mortar]
    friction_coefficient := 0.8
    friction_elastic_slip = ${elastic_slip}
  []
[]

[BCs]
  # Establish normal contact and elastic slip by t = 1, then co-rotate both bodies.
  active = 'primary_x primary_y primary_z secondary_x secondary_y secondary_z'
  [primary_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = bottom_bottom
    function = 'cos(if(t <= 1, 0, ${rotation_angle} * (t - 1))) * x - sin(if(t <= 1, 0, ${rotation_angle} * (t - 1))) * y - x'
  []
  [primary_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = bottom_bottom
    function = 'sin(if(t <= 1, 0, ${rotation_angle} * (t - 1))) * x + cos(if(t <= 1, 0, ${rotation_angle} * (t - 1))) * y - y'
  []
  [primary_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom_bottom
    value = 0
  []
  [secondary_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top_top
    function = 'cos(if(t <= 1, 0, ${rotation_angle} * (t - 1))) * (x + if(t <= 0.5, 0, if(t < 1, 2 * ${tangential_shift} * (t - 0.5), ${tangential_shift}))) - sin(if(t <= 1, 0, ${rotation_angle} * (t - 1))) * y - x'
  []
  [secondary_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top_top
    function = 'sin(if(t <= 1, 0, ${rotation_angle} * (t - 1))) * (x + if(t <= 0.5, 0, if(t < 1, 2 * ${tangential_shift} * (t - 0.5), ${tangential_shift}))) + cos(if(t <= 1, 0, ${rotation_angle} * (t - 1))) * y - y'
  []
  [secondary_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = top_top
    function = '-if(t < 0.5, 2 * ${compression} * t, ${compression})'
  []
[]

[Postprocessors]
  active := 'tangential_x_sum tangential_y_sum'
  [tangential_x_sum]
    type = NodalSum
    variable = mortar_tangent_x
    block = mortar_secondary_subdomain
    execute_on = 'initial timestep_end'
  []
  [tangential_y_sum]
    type = NodalSum
    variable = mortar_tangent_y
    block = mortar_secondary_subdomain
    execute_on = 'initial timestep_end'
  []
[]

[VectorPostprocessors]
  active = ''
[]

[Debug]
  show_var_residual_norms := false
[]

[Executioner]
  end_time := 2
  dt := 0.1
  dtmin := 0.1
  solve_type := NEWTON
  nl_max_its := 40
  nl_abs_tol := 1e-10
[]

[Outputs]
  exodus := false
  csv := false
  checkpoint = true
  [out]
    type = CSV
    time_step_interval = 10
  []
[]
