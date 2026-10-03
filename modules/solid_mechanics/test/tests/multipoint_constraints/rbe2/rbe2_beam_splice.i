[Mesh]
  allow_renumbering = false
  [segment_a]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 40
    xmin = 0.0
    xmax = 1.0
    elem_type = EDGE2
    subdomain_ids = '1'
    subdomain_name = 'segment_a'
  []
  [segment_a_nodesets]
    type = RenameBoundaryGenerator
    input = segment_a
    old_boundary = 'left right'
    new_boundary = 'fixed_end splice_independent'
  []
  [segment_b]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 40
    xmin = 1.0
    xmax = 2.0
    elem_type = EDGE2
    subdomain_ids = '2'
    subdomain_name = 'segment_b'
  []
  [segment_b_nodesets]
    type = RenameBoundaryGenerator
    input = segment_b
    old_boundary = 'left right'
    new_boundary = 'splice_dependent loaded_tip'
  []
  [combined]
    type = CombinerGenerator
    inputs = 'segment_a_nodesets segment_b_nodesets'
    avoid_merging_boundaries = true
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
  []
  [rot_y]
  []
  [rot_z]
  []
[]

[Physics/SolidMechanics/LineElement/QuasiStatic]
  [beam]
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 0.554256
    Ay = 0.0
    Az = 0.0
    Iy = 0.0141889
    Iz = 0.0141889
    y_orientation = '0.0 1.0 0.0'
  []
[]

[Materials]
  [elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 2.60072400269
    poissons_ratio = -0.9998699638
    shear_coefficient = 0.85
  []
  [stress]
    type = ComputeBeamResultants
  []
[]

[BCs]
  [fix_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'fixed_end'
    value = 0.0
  []
  [fix_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'fixed_end'
    value = 0.0
  []
  [fix_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'fixed_end'
    value = 0.0
  []
  [fix_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = 'fixed_end'
    value = 0.0
  []
  [fix_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'fixed_end'
    value = 0.0
  []
  [fix_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = 'fixed_end'
    value = 0.0
  []
[]

[NodalKernels]
  [tip_load]
    type = ConstantRate
    variable = disp_y
    boundary = 'loaded_tip'
    rate = 1.0e-4
  []
[]

[Constraints]
  [splice]
    type = RBE2Constraint
    independent_boundary = 'splice_independent'
    dependent_boundary = 'splice_dependent'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    dependent_dofs = translations_and_rotations
  []
[]

[Postprocessors]
  [tip_disp_y]
    type = PointValue
    point = '2.0 0.0 0.0'
    variable = disp_y
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
  line_search = 'none'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-11
  dt = 1.0
  num_steps = 2
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
