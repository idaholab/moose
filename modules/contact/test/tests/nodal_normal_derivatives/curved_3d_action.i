!include ../3d-mortar-contact/frictionless-mortar-3d-action.i

overclosure = 0.001

[Mesh]
  [top_curve]
    type = ParsedNodeTransformGenerator
    input = top_block_id
    x_function = x
    y_function = y
    z_function = 'z + 0.04 * x * x + 0.03 * y * y + 0.02 * x * y'
  []
  [bottom_curve]
    type = ParsedNodeTransformGenerator
    input = bottom_block_change_boundary_id
    x_function = x
    y_function = y
    z_function = 'z + 0.04 * x * x + 0.03 * y * y + 0.02 * x * y'
  []
  [combined]
    inputs := 'top_curve bottom_curve'
  []
  [Partitioner]
    type = GridPartitioner
    nx = 1
    ny = 1
    nz = 1
  []
[]

[ICs]
  [disp_z]
    value := '-${overclosure}'
  []
  [normal_lm]
    type = ConstantIC
    variable = mortar_normal_lm
    block = mortar_secondary_subdomain
    value = 1
  []
[]

[Contact]
  [mortar]
    model := coulomb
    friction_coefficient = 0.25
    use_dual = true
  []
[]

[BCs]
  active = 'botx boty botz move_top_x move_top_y move_top_z'
  [move_top_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top_top
    function = '0.001 * t'
  []
  [move_top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top_top
    function = '0.0015 * t'
  []
  [move_top_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = top_top
    function = '-${overclosure} * t'
  []
[]

[Executioner]
  solve_type := NEWTON
  end_time := 1
  dt := 1
  nl_abs_tol := 1e-10
  nl_rel_tol := 1e-8
  nl_max_its := 25
  line_search := none
[]

[Postprocessors]
  active := num_nl
[]

[VectorPostprocessors]
  active = ''
[]

[Outputs]
  exodus := false
[]
