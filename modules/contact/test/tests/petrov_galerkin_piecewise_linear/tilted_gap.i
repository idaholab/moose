# The top block is tilted so the gap to the bottom block grows linearly along the interface, and the
# blocks are not loaded. The nodal weighted gap then depends on the basis that weights it: with the
# piecewise-linear basis of the EDGE3 faces, an end node of the interface weights the gap only over
# its half of the end face.
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
    xmax = 1
    ymin = 0.01
    ymax = 0.5
    elem_type = QUAD8
    boundary_name_prefix = top
    boundary_id_offset = 10
  []
  # Gap from 0.01 at x = 0 to 0.03 at x = 1, linear along the bottom face of the top block
  [tilt]
    type = ParsedNodeTransformGenerator
    input = top
    x_function = 'x'
    y_function = 'y + 0.02 * x * (0.5 - y) / 0.49'
    z_function = 'z'
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
    primary = bottom_top
    secondary = top_bottom
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
    boundary = 'bottom_bottom top_top'
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

[VectorPostprocessors]
  [weighted_gap]
    type = NodalValueSampler
    variable = weighted_gap
    block = mortar_secondary_subdomain
    sort_by = x
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
