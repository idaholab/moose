!include frictional-nodal-min-normal-lm-mortar-pdass-tangential-lm-mortar-disp.i
[Constraints]
  [frictional_normal_lm]
    epsilon = 0
    friction_elastic_slip = 1e-4
  []
[]

[Functions]
  [normal_loading]
    type = PiecewiseLinear
    x = '0 1 4'
    y = '${fparse starting_point + offset} ${fparse -starting_point + offset} ${fparse -starting_point + offset}'
  []
  [tangential_loading]
    type = PiecewiseLinear
    x = '0 1 2 3 4 5'
    y = '0 0 2e-4 2e-4 0 2e-4'
  []
[]

[BCs]
  [topy]
    function := normal_loading
  []
  [leftx]
    function := tangential_loading
  []
[]

[Executioner]
  end_time := 5
  dt := 0.1
  dtmin := 0.1
  nl_abs_tol = 1e-12
[]

[Debug]
  show_var_residual_norms := false
[]

[Postprocessors]
  active = 'tangential_lm_sum normal_lm_sum'
  [tangential_lm_sum]
    type = NodalSum
    variable = frictional_tangential_lm
    block = 3
    execute_on = 'initial timestep_end'
  []
  [normal_lm_sum]
    type = NodalSum
    variable = frictional_normal_lm
    block = 3
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  exodus := false
  checkpoint = true
  [out]
    type = CSV
  []
[]
