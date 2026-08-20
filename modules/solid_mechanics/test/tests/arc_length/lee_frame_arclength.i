leg = 120
thick = 2

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [column]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 236
    xmax = ${thick}
    ymax = ${fparse leg - thick}
    elem_type = QUAD4
    boundary_name_prefix = column
  []
  [beam]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 240
    ny = 4
    xmax = ${leg}
    ymin = ${fparse leg - thick}
    ymax = ${leg}
    elem_type = QUAD4
    boundary_name_prefix = beam
  []
  [frame]
    type = StitchMeshGenerator
    inputs = 'column beam'
    stitch_boundaries_pairs = 'column_top beam_bottom'
  []
  [base_patch]
    type = SubdomainBoundingBoxGenerator
    input = frame
    block_id = 2
    bottom_left = '0 0 0'
    top_right = '${thick} ${thick} 0'
  []
  [end_patch]
    type = SubdomainBoundingBoxGenerator
    input = base_patch
    block_id = 2
    bottom_left = '${fparse leg - thick} ${fparse leg - thick} 0'
    top_right = '${leg} ${leg} 0'
  []
  [load_patch]
    type = SubdomainBoundingBoxGenerator
    input = end_patch
    block_id = 2
    bottom_left = '${fparse thick + 23} ${fparse leg - thick} 0'
    top_right = '${fparse thick + 25} ${leg} 0'
  []
  [column_hinge]
    type = ExtraNodesetGenerator
    input = load_patch
    new_boundary = column_hinge
    coord = '${fparse thick / 2} 0 0'
  []
  [beam_hinge]
    type = ExtraNodesetGenerator
    input = column_hinge
    new_boundary = beam_hinge
    coord = '${leg} ${fparse leg - thick / 2} 0'
  []
  [load_face]
    type = ParsedGenerateSideset
    input = beam_hinge
    combinatorial_geometry = 'y > ${fparse leg - 1e-9} & x > ${fparse thick + 23 - 1e-9} & x < ${fparse thick + 25 + 1e-9}'
    normal = '0 1 0'
    new_sideset_name = load_face
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    decomposition_method = TaylorExpansion
    use_automatic_differentiation = true
    add_variables = true
  []
[]

[BCs]
  [beam_load]
    type = NeumannBC
    variable = disp_y
    boundary = load_face
    value = ${fparse -5 / 2}
    vector_tags = 'arc_length_load'
  []
  [column_hinge_x]
    type = DirichletBC
    variable = disp_x
    boundary = column_hinge
    value = 0
  []
  [column_hinge_y]
    type = DirichletBC
    variable = disp_y
    boundary = column_hinge
    value = 0
  []
  [beam_hinge_x]
    type = DirichletBC
    variable = disp_x
    boundary = beam_hinge
    value = 0
  []
  [beam_hinge_y]
    type = DirichletBC
    variable = disp_y
    boundary = beam_hinge
    value = 0
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 71240
    poissons_ratio = 0.3
    block = 0
  []
  [elasticity_patches]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 7.124e6
    poissons_ratio = 0.3
    block = 2
  []
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
[]

[Problem]
  type = ArcLengthProblem
  step_size = 10
  psi_squared = 0
  correction_type = normal
[]

[Postprocessors]
  [lambda]
    type = ArcLengthLoadParameter
  []
  [P]
    type = ScalePostprocessor
    value = lambda
    scaling_factor = 5
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [load_point_v]
    type = PointValue
    variable = disp_y
    point = '${fparse thick + 24} ${leg} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
  [joint_sway]
    type = PointValue
    variable = disp_x
    point = '${fparse thick / 2} ${fparse leg - thick / 2} 0'
    execute_on = 'ARC_LENGTH_INCREMENT TIMESTEP_END'
  []
[]

[VectorPostprocessors]
  [path]
    type = ArcLengthHistory
    postprocessors = 'load_point_v joint_sway'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none
  automatic_scaling = true
  dt = 0.1
  end_time = 80
  dtmin = 1e-12
  nl_max_its = 50
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  exodus = true
[]
