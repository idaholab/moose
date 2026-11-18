E = 1e3
poi = 0.3
interfaces = 'grain1_grain2 grain1_grain4 grain1_grain5 grain2_grain4 grain3_grain4 grain3_grain5 grain4_grain5'

ny = 6
nx = '${fparse ny*3/2}'

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [boundary_mesh]
    type = FileMeshGenerator
    file = 'grain_boundary_only.msh'
  []

  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.6
    xmax = 0.6
    ymin = -0.4
    ymax = 0.4
    nx = ${nx}
    ny = ${ny}
  []

  [grain_gen]
    type = SubdomainGrainIDGenerator
    input = gen
    boundary_mesh = boundary_mesh
    lambda = 1
  []

  [break7]
    input = grain_gen
    type = BreakMeshByBlockGenerator
    split_interface = true
    add_interface_on_two_sides = true
  []

  [left_bottom]
    input = break7
    type = ExtraNodesetGenerator
    coord = '-0.6 -0.4'
    new_boundary = 'left_bottom'
  []

  [right_top]
    input = left_bottom
    type = ExtraNodesetGenerator
    coord = '0.6 0.4'
    new_boundary = 'right_top'
  []

  [grain1_grain2]
    type = FileMeshGenerator
    file = 'grain1_grain2.msh'
    save_with_name = 'grain1_grain2'
  []

  [grain1_grain4]
    type = FileMeshGenerator
    file = 'grain1_grain4.msh'
    save_with_name = 'grain1_grain4'
  []

  [grain1_grain5]
    type = FileMeshGenerator
    file = 'grain1_grain5.msh'
    save_with_name = 'grain1_grain5'
  []

  [grain2_grain4]
    type = FileMeshGenerator
    file = 'grain2_grain4.msh'
    save_with_name = 'grain2_grain4'
  []

  [grain3_grain4]
    type = FileMeshGenerator
    file = 'grain3_grain4.msh'
    save_with_name = 'grain3_grain4'
  []

  [grain3_grain5]
    type = FileMeshGenerator
    file = 'grain3_grain5.msh'
    save_with_name = 'grain3_grain5'
  []

  [grain4_grain5]
    type = FileMeshGenerator
    file = 'grain4_grain5.msh'
    save_with_name = 'grain4_grain5'
  []

  add_subdomain_ids = '10'
  add_subdomain_names = 'block_10'
  final_generator = 'right_top'
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
        extra_vector_tags = 'ref'
        use_automatic_differentiation = false
        generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy strain_zz'
      []
    []
  []
[]
[Physics/SolidMechanics/ShiftedCohesiveZone]
  [czm_ik]
    use_automatic_differentiation = false
    boundary = ${interfaces}
    generate_sbm_distance = true
    check_surface_watertightness = true
  []
[]

[Materials]
  [elastic_stress]
    type = ComputeLinearElasticStress
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = ${poi}
    youngs_modulus = ${E}
  []
  [interface_traction]
    type = BiLinearMixedModeTraction
    boundary = ${interfaces}
    penalty_stiffness = 2e3
    GI_c = 1e3
    GII_c = 1e2
    normal_strength = 500
    shear_strength = 300
    displacements = 'disp_x disp_y'
    eta = 2.2
    viscosity = 1e-3
  []
[]

[Executioner]
  type = Transient
  # solve_type = FD
  # automatic_scaling = false
  # residual_and_jacobian_together = false
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
  automatic_scaling = true
  # residual_and_jacobian_together = true
  line_search = 'none'

  nl_max_its = 500
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-50
  dt = 1
  end_time = 5
[]

[Functions]
  [displacement_with_time]
    type = ParsedFunction
    expression = '1e-2*t'
  []
[]

[Kernels]
[]

[AuxVariables]
  [react_x]
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
  [react_x]
    type = NodalSum
    variable = 'react_x'
    boundary = 'right'
  []

  [area]
    type = ElementIntegralFunctorPostprocessor
    functor = 1
    block = '1 2'
    execute_on = 'final'
  []
[]

[Outputs]
  exodus = true
  # execute_on = 'final'
  [csv]
    type = CSV
    precision = 15
  []
[]
