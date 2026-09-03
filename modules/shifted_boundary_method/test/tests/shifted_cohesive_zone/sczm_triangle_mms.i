E1 = 0.1
E2 = 1
poi = 0
x0 = 0.25
a = '${fparse (pi * cos(pi * x0) * (1 - E1 / E2)) / (2 * x0)}'
b = '${fparse -pi * cos(pi * x0) * (E1 + (x0 / 2) * (1 - E1 / E2))}'

desired_area = 0.01

number_of_point = '${fparse int(sqrt(1/(2*desired_area))) -1}'

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
  sbm_distance_uo = sbm_distance_uo
[]

[Mesh]
  [square_boundary]
    type = PolyLineMeshGenerator
    points = '-0.5 0.0 0.0
            0.5 0.0 0.0
            0.5 1.0 0.0
            -0.5 1.0 0.0'
    loop = true
  []

  [gen]
    type = XYDelaunayGenerator
    boundary = 'square_boundary'
    desired_area = ${desired_area}
    add_nodes_per_boundary_segment = ${number_of_point}
    refine_boundary = false
  []

  [subdomain_intercepted]
    type = SubdomainInterceptedGenerator
    input = gen
    subdomain_id_inside = 1
    subdomain_id_outside = 2
    lambda = 0.5
    is_domain_inside_surface = false
    signed_dist_function = '-x+${x0}'
  []

  [break]
    type = BreakMeshByBlockGenerator
    block_pairs = '1 2'
    input = subdomain_intercepted
    split_interface = true
    add_interface_on_two_sides = true
  []

  [ss]
    type = SideSetsFromNormalsGenerator
    input = break
    normals = '-1 0 0
                1 0 0'
    fixed_normal = true
    new_boundary = 'left right'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        use_automatic_differentiation = true
        generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
      []
    []
  []
[]

[Physics/SolidMechanics/ShiftedCohesiveZone]
  [czm_ik]
    use_automatic_differentiation = true
    boundary = 'Block1_Block2'
  []
[]

[UserObjects]
  [sbm_distance_uo]
    type = BoundaryShortestDistanceToSurface
    surfaces = 'signed_dist_func'
    boundary = 'Block1_Block2'
    execution_order_group = 0
    execute_on = 'INITIAL'
  []
[]

[Materials]
  [elastic_stress]
    type = ADComputeLinearElasticStress
  []
  [elasticity_tensor1]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E1}
    block = 1
  []
  [elasticity_tensor2]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E2}
    block = 2
  []
  [interface_traction]
    type = ADPureElasticTractionSeparation
    normal_stiffness = 1
    tangent_stiffness = 1
    boundary = 'Block1_Block2'
  []
[]

[AuxVariables]
  [a]
    family = MONOMIAL
  []
[]

[AuxKernels]
  [a]
    type = ConstantAux
    variable = a
    value = ${a}
  []
[]

[Kernels]
  [source_x_left]
    type = BodyForce
    variable = disp_x
    function = bx_func_left
    block = 1
  []
  [source_x_right]
    type = BodyForce
    variable = disp_x
    function = bx_func_right
    block = 2
  []
[]

[Functions]
  [signed_dist_func]
    type = ParsedFunction
    expression = '-x+${x0}'
  []
  [ux_func_left]
    type = ParsedFunction
    expression = '-sin(pi*x)'
  []

  [ux_func_right]
    type = ParsedFunction
    expression = '-sin(pi*x)+${a}*x^2+${b}'
  []

  [bx_func_left]
    type = ParsedFunction
    expression = '-pi^2*${E1}*sin(pi*x)'
  []

  [bx_func_right]
    type = ParsedFunction
    expression = '-${E2}*(2*${a} + pi^2*sin(pi*x))'
  []
[]

[BCs]
  [y_anchor]
    type = DirichletBC
    variable = disp_y
    boundary = 'left right'
    value = 0.0
  []

  [x_left]
    type = FunctionDirichletBC
    boundary = 'left'
    function = ux_func_left
    variable = disp_x
  []

  [x_right]
    type = FunctionDirichletBC
    boundary = 'right'
    function = ux_func_right
    variable = disp_x
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  automatic_scaling = true
  line_search = 'none'
[]

[Postprocessors]
  [l2_error_left_x]
    type = ElementL2Error
    variable = disp_x
    function = ux_func_left
    block = '1'
  []

  [l2_error_right_x]
    type = ElementL2Error
    variable = disp_x
    function = ux_func_right
    block = '2'
  []

  [l2_sum]
    type = SumPostprocessor
    values = 'l2_error_left_x l2_error_right_x'
  []

  [n_elements]
    type = NumElements
  []
[]

[Outputs]
  exodus = true
[]
