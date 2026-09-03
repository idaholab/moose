nx = 8
nx_half = '${fparse nx / 2}'
E = 1e3
poi = 0.3
theta = 60
theta_rad = '${fparse theta * pi / 180}'
x0 = 0.5

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
  sbm_distance_uo = sbm_distance_uo
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${x0} ${x0}'
    dy = 1
    ix = '${nx_half} ${nx_half}'
    iy = '${nx}'
    subdomain_id = '1 2'
  []
  [intercepted]
    type = SubdomainInterceptedGenerator
    input = gen
    subdomain_id_inside = 2
    subdomain_id_outside = 1
    lambda = 0.5
    is_domain_inside_surface = true
    signed_dist_function = '(x-${x0})*sin(${theta_rad}) - cos(${theta_rad})*(y-${x0})'
  []
  [break]
    type = BreakMeshByBlockGenerator
    input = intercepted
    split_interface = true
    add_interface_on_two_sides = true
  []
[]

[UserObjects]
  [sbm_distance_uo]
    type = BoundaryShortestDistanceToSurface
    surfaces = signed_dist_func
    boundary = 'Block1_Block2 Block2_Block1'
    execution_order_group = 0
    execute_on = INITIAL
  []
[]

[Functions]
  [displacement_with_time]
    type = ParsedFunction
    expression = '1e-2*t'
  []
  [signed_dist_func]
    type = ParsedFunction
    expression = '(x-${x0})*sin(${theta_rad}) - cos(${theta_rad})*(y-${x0})'
  []
[]

[AuxVariables]
  [react_x]
  []
[]

[AuxKernels]
  [react_x]
    type = TagVectorAux
    vector_tag = ref
    v = disp_x
    variable = react_x
    remove_variable_scaling = true
  []
[]

[BCs]
  [anchor_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [anchor_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [pull]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = displacement_with_time
    preset = false
  []
[]

[Postprocessors]
  [react_x]
    type = NodalSum
    variable = react_x
    boundary = right
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  automatic_scaling = true
  line_search = none
  nl_max_its = 50
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-10
  dt = 1
  end_time = 5
  abort_on_solve_fail = true
[]

[Outputs]
  [csv]
    type = CSV
    precision = 15
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        extra_vector_tags = ref
        add_variables = true
        use_automatic_differentiation = true
      []
    []
  []
[]

[Physics/SolidMechanics/ShiftedCohesiveZone]
  [czm]
    use_automatic_differentiation = true
    boundary = Block1_Block2
  []
[]

[Materials]
  [elastic_stress]
    type = ADComputeLinearElasticStress
  []
  [elasticity_tensor_1]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E}
    block = 1
  []
  [elasticity_tensor_2]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E}
    block = 2
  []
  [czm]
    type = ADPureElasticTractionSeparation
    normal_stiffness = 5000
    tangent_stiffness = 5000
    boundary = Block1_Block2
  []
[]
