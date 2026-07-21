starting_point = 0.04

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1
  scaling = 1
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.3
    xmax = 0
    ymin = -0.5
    ymax = 0.5
    nx = 2
    ny = 3
    elem_type = QUAD4
  []
  [left_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [left_curve]
    type = ParsedNodeTransformGenerator
    input = left_sidesets
    x_function = 'x + 0.08 * y * y'
    y_function = 'y'
    z_function = 'z'
  []
  [left_id]
    type = SubdomainIDGenerator
    input = left_curve
    subdomain_id = 1
  []

  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.3
    ymin = -0.5
    ymax = 0.5
    nx = 2
    ny = 3
    elem_type = QUAD4
  []
  [right_sidesets]
    type = RenameBoundaryGenerator
    input = right_block
    old_boundary = '0 1 2 3'
    new_boundary = '20 21 22 23'
  []
  [right_curve]
    type = ParsedNodeTransformGenerator
    input = right_sidesets
    x_function = 'x + 0.08 * y * y'
    y_function = 'y'
    z_function = 'z'
  []
  [right_id]
    type = SubdomainIDGenerator
    input = right_curve
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'left_id right_id'
  []
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
[]

[Kernels]
  [disp_x]
    type = MatDiffusion
    variable = disp_x
  []
  [disp_y]
    type = MatDiffusion
    variable = disp_y
  []
[]

[ICs]
  [overclose_secondary]
    type = ConstantIC
    variable = disp_x
    block = 1
    value = ${starting_point}
  []
  [normal_lm]
    type = ConstantIC
    variable = mortar_normal_lm
    block = 'mortar_secondary_subdomain'
    value = 1
  []
[]

[Contact]
  [mortar]
    primary = '23'
    secondary = '11'
    formulation = mortar
    model = frictionless
    c_normal = 1e2
    use_dual = false
  []
[]

[UserObjects]
  [verify_nodal_normal_derivatives]
    type = NodalNormalDerivativesTest
    weighted_gap_uo = lm_weightedgap_object_mortar
    disp_x = disp_x
    disp_y = disp_y
  []
[]

[BCs]
  [push_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = '13'
    function = '${starting_point} * t'
  []
  [pin_left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '13'
    function = 0
  []
  [fix_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = '21'
    value = 0
  []
  [fix_right_y]
    type = DirichletBC
    variable = disp_y
    boundary = '21'
    value = 0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 1
  dt = 1
  nl_abs_tol = 1e-11
  nl_rel_tol = 1e-9
  nl_max_its = 20
  l_max_its = 50
  line_search = none
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO 1e-15'
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = false
  csv = false
[]
