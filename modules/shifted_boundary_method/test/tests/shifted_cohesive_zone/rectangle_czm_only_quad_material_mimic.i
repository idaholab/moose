E1 = 1e3
E2 = 1e3
poi = 0.3

x0 = 0.5

dt = 1

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
    elem_type = QUAD4
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
[Physics/SolidMechanics/CohesiveZone]
  [czm_ik]
    use_automatic_differentiation = false
    boundary = 'Block1_Block2'
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
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    precision = 15
  []
[]
