E = 1e3
poi = 0.3

ny = 6
nx = '${fparse ny*3/2}'

[GlobalParams]
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
  sbm_distance_uo = sbm_distance_uo
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

  [boundary_mesh2]
    type = FileMeshGenerator
    file = 'grain_boundary_only.msh'
    save_with_name = 'boundary_mesh2'
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
    boundary = 'grain1_grain2 grain1_grain4 grain1_grain5 grain2_grain4 grain3_grain4 grain3_grain5 grain4_grain5'
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
    boundary = 'grain1_grain2 grain1_grain4 grain1_grain5 grain2_grain4 grain3_grain4 grain3_grain5 grain4_grain5'
    penalty_stiffness = 2e3
    GI_c = 1e3
    GII_c = 1e2
    normal_strength = 500
    shear_strength = 300
    displacements = 'disp_x disp_y'
    eta = 2.2
    viscosity = 1e-3
  []
  [gc_integral]
    type = CZMGcIntegral
    boundary = 'grain1_grain2 grain1_grain4 grain1_grain5 grain2_grain4 grain3_grain4 grain3_grain5 grain4_grain5'
  []
[]

[UserObjects]
  [grain1_grain2_builder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = grain1_grain2
  []

  [grain1_grain4_builder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = grain1_grain4
  []

  [grain1_grain5_builder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = grain1_grain5
  []

  [grain2_grain4_builder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = grain2_grain4
  []

  [grain3_grain4_builder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = grain3_grain4
  []

  [grain3_grain5_builder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = grain3_grain5
  []

  [grain4_grain5_builder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = grain4_grain5
  []

  [SBMMeshBySubBuilder]
    type = SurfaceMeshBySubdomainBuilder
    surface_mesh = boundary_mesh2
  []

  [point_in_subdomain_check]
    type = PointInSubdomainCheckUO
    builder = SBMMeshBySubBuilder
  []

  [sbm_distance_uo]
    type = BoundaryShortestDistanceToSurface
    surfaces = 'dist_grain1_grain2 dist_grain1_grain4 dist_grain1_grain5 dist_grain2_grain4 dist_grain3_grain4 dist_grain3_grain5 dist_grain4_grain5'
    boundary = 'grain1_grain2 grain1_grain4 grain1_grain5 grain2_grain4 grain3_grain4 grain3_grain5 grain4_grain5'
    execution_order_group = 0
    execute_on = 'INITIAL'
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
  end_time = 200
  abort_on_solve_fail = true
[]

[Functions]
  [dist_grain1_grain2]
    type = UnsignedDistanceToSurfaceMesh
    builder = grain1_grain2_builder
  []
  [dist_grain1_grain4]
    type = UnsignedDistanceToSurfaceMesh
    builder = grain1_grain4_builder
  []
  [dist_grain1_grain5]
    type = UnsignedDistanceToSurfaceMesh
    builder = grain1_grain5_builder
  []
  [dist_grain2_grain4]
    type = UnsignedDistanceToSurfaceMesh
    builder = grain2_grain4_builder
  []
  [dist_grain3_grain4]
    type = UnsignedDistanceToSurfaceMesh
    builder = grain3_grain4_builder
  []
  [dist_grain3_grain5]
    type = UnsignedDistanceToSurfaceMesh
    builder = grain3_grain5_builder
  []
  [dist_grain4_grain5]
    type = UnsignedDistanceToSurfaceMesh
    builder = grain4_grain5_builder
  []
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
    scaled = false
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

  [gc_corrected]
    type = AreaCorrectedSideIntegralMaterialProperty
    boundary = 'grain1_grain2 grain1_grain4 grain1_grain5 grain2_grain4 grain3_grain4 grain3_grain5 grain4_grain5'
    property = gc_integral
    execute_on = 'TIMESTEP_END'
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
