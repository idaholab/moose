E1 = 1e3
E2 = 1e3
poi = 0.3

desired_area = 0.1
number_of_point = '${fparse int(sqrt(1/(2*desired_area))) -1}'

x0 = 0.5

dt = 1

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
  sbm_distance_uo = sbm_distance_uo
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [square_boundary]
    type = PolyLineMeshGenerator
    points = '0.0 0.0 0.0
            1.0 0.0 0.0
            1.0 1.0 0.0
            0.0 1.0 0.0'
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
    is_domain_inside_surface = true
    signed_dist_function = 'x-${x0}'
  []

  [break]
    type = BreakMeshByBlockGenerator
    input = subdomain_intercepted
    split_interface = true
    add_interface_on_two_sides = true
  []

  [ss]
    type = SideSetsFromNormalsGenerator
    input = break
    normals = '-1 0 0
                1 0 0
                0 -1 0'
    fixed_normal = true
    new_boundary = 'left right bottom'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        extra_vector_tags = 'ref'
        add_variables = true
        use_automatic_differentiation = false
        generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
      []
    []
  []
[]
[Physics/SolidMechanics/ShiftedCohesiveZone]
  [czm_ik]
    use_automatic_differentiation = false
    boundary = 'Block1_Block2'
  []
[]

[UserObjects]
  [sbm_distance_uo]
    type = BoundaryShortestDistanceToSurface
    surfaces = 'signed_dist_func'
    boundary = 'Block1_Block2 Block2_Block1'
    execution_order_group = 0
    execute_on = 'INITIAL'
    # The coarse test mesh yields surrogate distances comparable to the element size.
    suppress_distance_warning = true
  []
[]

[Materials]
  [elastic_stress]
    type = ComputeLinearElasticStress
  []
  [elasticity_tensor_in]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E1}
    block = 1
  []
  [elasticity_tensor_out]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E2}
    block = 2
  []
  [czm]
    type = SalehaniIrani3DCTraction
    normal_gap_at_maximum_normal_traction = 1
    tangential_gap_at_maximum_shear_traction = 0.5
    maximum_normal_traction = 500
    maximum_shear_traction = 300
    boundary = 'Block1_Block2'
  []
  [gc_integral]
    type = CZMGcIntegral
    boundary = 'Block1_Block2'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  automatic_scaling = true
  line_search = 'none'

  nl_max_its = 500
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-50
  dt = ${dt}
  end_time = 5
  abort_on_solve_fail = true
[]

[Functions]
  [displacement_with_time]
    type = ParsedFunction
    expression = '1e-2*t'
  []
  [signed_dist_func]
    type = ParsedFunction
    expression = 'x-${x0}'
  []
  [const_1]
    type = ConstantFunction
    value = 1
  []
[]

[AuxVariables]
  [react_x]
  []
  [react_y]
  []
[]

[AuxKernels]
  [react_x]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_x'
    variable = 'react_x'
    remove_variable_scaling = true
  []
  [react_y]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_y'
    variable = 'react_y'
    remove_variable_scaling = true
  []
[]

[BCs]
  [anchor_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  []
  [anchor_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  []
  [displacement_x_right]
    #Anchors the left side against deformation in the x-direction
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'right'
    function = displacement_with_time
    preset = false
  []
[]

[Postprocessors]
  [area]
    type = ElementIntegralFunctorPostprocessor
    functor = 1
    block = '1 2'
    execute_on = 'final'
  []

  [n_elements]
    type = NumElements
  []

  [sqroot_1_div_n_elements]
    type = ParsedPostprocessor
    expression = 'sqrt(1 / n_elements)'
    pp_names = 'n_elements'
  []

  [react_x]
    type = NodalSum
    variable = 'react_x'
    boundary = 'right'
  []
  [react_y]
    type = NodalSum
    variable = 'react_y'
    boundary = 'right'
  []
  [length]
    type = AreaPostprocessor
    boundary = 'right'
  []

  [left_distance_integral]
    type = SideIntegralShiftedVariablePostprocessor
    variable = disp_x
    boundary = 'Block1_Block2'
  []

  [right_distance_integral]
    type = SideIntegralShiftedVariablePostprocessor
    variable = disp_x
    boundary = 'Block2_Block1'
  []

  [gc]
    type = SideIntegralMaterialProperty
    boundary = 'Block1_Block2'
    property = gc_integral
    execute_on = 'TIMESTEP_END'
  []

  [gc_corrected]
    type = AreaCorrectedSideIntegralMaterialProperty
    boundary = 'Block1_Block2'
    property = gc_integral
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    precision = 15
  []
[]
