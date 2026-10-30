# The top block is tilted so the gap to the bottom block grows linearly in x, and the blocks are not
# loaded. The nodal weighted gap then depends on the basis that weights it: with the
# piecewise-linear basis of the second-order faces, a node on the x = 0 edge of the interface
# weights the gap only over the sub-elements next to it.
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [bottom]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmax = 1
    ymax = 1
    zmin = -0.5
    zmax = 0
    elem_type = HEX20
    boundary_name_prefix = bottom
  []
  [bottom_id]
    type = SubdomainIDGenerator
    input = bottom
    subdomain_id = 1
  []
  [top]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 1
    nz = 1
    xmax = 1
    ymax = 1
    zmin = 0.01
    zmax = 0.5
    elem_type = HEX20
    boundary_name_prefix = top
    boundary_id_offset = 10
  []
  # Gap from 0.01 at x = 0 to 0.03 at x = 1, linear on the bottom face of the top block
  [tilt]
    type = ParsedNodeTransformGenerator
    input = top
    x_function = 'x'
    y_function = 'y'
    z_function = 'z + 0.02 * x * (0.5 - z) / 0.49'
  []
  [top_id]
    type = SubdomainIDGenerator
    input = tilt
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
    primary = bottom_front
    secondary = top_back
    formulation = mortar
    model = frictionless
    c_normal = 1e4
    use_petrov_galerkin = true
    petrov_galerkin_test = PIECEWISE_LINEAR
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'bottom_left top_left'
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom_bottom top_bottom'
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'bottom_back top_front'
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e3
    poissons_ratio = 0
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  []
[]

[AuxVariables]
  [weighted_gap]
    block = mortar_secondary_subdomain
    order = SECOND
  []
[]

[AuxKernels]
  [weighted_gap]
    type = MortarUserObjectAux
    variable = weighted_gap
    contact_quantity = normal_gap
    user_object = lm_weightedgap_object_mortar
  []
[]

[Postprocessors]
  # Corner node of the interface on the x = 0 edge, where the gap is 0.01
  [corner_weighted_gap]
    type = PointValue
    variable = weighted_gap
    point = '0 0 0.01'
  []
  # Mid-edge node on the x = 0 edge
  [mid_edge_weighted_gap]
    type = PointValue
    variable = weighted_gap
    point = '0 0.5 0.01'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
