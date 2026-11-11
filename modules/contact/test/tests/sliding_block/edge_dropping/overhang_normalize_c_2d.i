# The secondary (top) block is pressed onto the primary block and slid while the left EDGE3 face of
# its contact surface overhangs the primary edge by more than a quarter of its length. The covered
# nodal integral of that face's end node test function is negative there.
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [bottom]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    xmax = 1
    ymin = -0.5
    ymax = 0
    elem_type = QUAD8
    boundary_name_prefix = bottom
  []
  [bottom_id]
    type = SubdomainIDGenerator
    input = bottom
    subdomain_id = 1
  []
  [top]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 1
    xmin = -0.125
    xmax = 0.875
    ymin = 0
    ymax = 0.5
    elem_type = QUAD8
    boundary_name_prefix = top
    boundary_id_offset = 10
  []
  [top_id]
    type = SubdomainIDGenerator
    input = top
    subdomain_id = 2
  []
  [combined]
    type = CombinerGenerator
    inputs = 'bottom_id top_id'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    block = '1 2'
  []
[]

[Contact]
  [mortar]
    normalize_c = true
    primary = bottom_top
    secondary = top_bottom
    formulation = mortar
    model = frictionless
    c_normal = 1e4
    correct_edge_dropping = true
  []
[]

[Functions]
  [push]
    type = PiecewiseLinear
    x = '0 1 5'
    y = '0 -0.01 -0.01'
  []
  [slide]
    type = PiecewiseLinear
    x = '0 1 5'
    y = '0 0 0.2'
  []
[]

[BCs]
  [fix_bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom_bottom
    value = 0
  []
  [fix_bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom_bottom
    value = 0
  []
  [push]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top_top
    function = push
  []
  [slide]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top_top
    function = slide
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  []
[]





[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
  line_search = none
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  nl_max_its = 30
  dt = 0.5
  end_time = 1.5
[]


[Postprocessors]
  [min_lm]
    type = NodalExtremeValue
    variable = mortar_normal_lm
    block = mortar_secondary_subdomain
    value_type = min
  []
  [max_lm]
    type = NodalExtremeValue
    variable = mortar_normal_lm
    block = mortar_secondary_subdomain
  []
[]

[Outputs]
  csv = true
[]
