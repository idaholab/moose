overclosure = 0.001

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  diffusivity = 1
  scaling = 1
[]

[Mesh]
  [top]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = 0
    zmax = 0.2
    nx = 2
    ny = 2
    nz = 1
    elem_type = HEX8
  []
  [top_sidesets]
    type = RenameBoundaryGenerator
    input = top
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '10 11 12 13 14 15'
  []
  [top_curve]
    type = ParsedNodeTransformGenerator
    input = top_sidesets
    x_function = x
    y_function = y
    z_function = 'z + 0.04 * x * x + 0.03 * y * y + 0.02 * x * y'
  []
  [top_id]
    type = SubdomainIDGenerator
    input = top_curve
    subdomain_id = 1
  []

  [bottom]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.6
    xmax = 0.6
    ymin = -0.6
    ymax = 0.6
    zmin = -0.2
    zmax = 0
    nx = 2
    ny = 2
    nz = 1
    elem_type = HEX8
  []
  [bottom_sidesets]
    type = RenameBoundaryGenerator
    input = bottom
    old_boundary = '0 1 2 3 4 5'
    new_boundary = '20 21 22 23 24 25'
  []
  [bottom_curve]
    type = ParsedNodeTransformGenerator
    input = bottom_sidesets
    x_function = x
    y_function = y
    z_function = 'z + 0.04 * x * x + 0.03 * y * y + 0.02 * x * y'
  []
  [bottom_id]
    type = SubdomainIDGenerator
    input = bottom_curve
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'top_id bottom_id'
  []
  [Partitioner]
    type = GridPartitioner
    nx = 1
    ny = 1
    nz = 1
  []
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
  [disp_z]
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
  [disp_z]
    type = MatDiffusion
    variable = disp_z
  []
[]

[ICs]
  [overclose_top]
    type = ConstantIC
    variable = disp_z
    block = 1
    value = '-${overclosure}'
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
    primary = 25
    secondary = 10
    formulation = mortar
    model = coulomb
    friction_coefficient = 0.25
    use_dual = true
  []
[]

[UserObjects]
  [verify_nodal_normal_derivatives]
    type = NodalNormalDerivativesTest
    weighted_gap_uo = lm_weightedvelocities_object_mortar
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  []
[]

[BCs]
  [move_top_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 15
    function = '0.001 * t'
  []
  [move_top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 15
    function = '0.0015 * t'
  []
  [move_top_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 15
    function = '-${overclosure} * t'
  []
  [fix_bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = '20 21 22 23 24'
    value = 0
  []
  [fix_bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = '20 21 22 23 24'
    value = 0
  []
  [fix_bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = '20 21 22 23 24'
    value = 0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 1
  dt = 1
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8
  nl_max_its = 25
  l_max_its = 60
  line_search = none
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO 1e-15'
[]

[Outputs]
  exodus = false
  csv = false
[]
