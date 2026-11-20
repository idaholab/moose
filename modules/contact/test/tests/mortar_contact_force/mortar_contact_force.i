# Two tall blocks on a wider bottom block, with all three contact pairs in one Contact block. The
# force of each bottom pair is compared with the nodal sum of the contact residual on its secondary
# surface. The two bottom pairs share the primary surface base_top.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [bottom_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 2
    ymax = 1
    nx = 5
    ny = 2
    subdomain_ids = 1
    boundary_name_prefix = base
    boundary_id_offset = 10
  []
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymin = 1
    ymax = 3
    nx = 3
    ny = 4
    subdomain_ids = 2
    boundary_name_prefix = left
    boundary_id_offset = 20
  []
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1
    xmax = 2
    ymin = 1
    ymax = 3
    nx = 2
    ny = 3
    subdomain_ids = 3
    boundary_name_prefix = right
    boundary_id_offset = 30
  []
  [combine]
    type = CombinerGenerator
    inputs = 'bottom_block left_block right_block'
  []
[]

[Problem]
  extra_tag_vectors = 'contact_force'
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = SMALL
    block = '1 2 3'
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    block = '1 2 3'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
    block = '1 2 3'
  []
[]

[Contact]
  active = 'mortar'
  [mortar]
    primary = 'base_top base_top right_left'
    secondary = 'left_bottom right_bottom left_right'
    model = frictionless
    formulation = mortar
    correct_edge_dropping = true
    extra_vector_tags = 'contact_force'
  []
  [penalty]
    primary = 'base_top base_top right_left'
    secondary = 'left_bottom right_bottom left_right'
    model = frictionless
    formulation = mortar_penalty
    penalty = 1e7
    extra_vector_tags = 'contact_force'
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = base_bottom
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = base_bottom
    value = 0
  []
  [confine_left]
    type = DirichletBC
    variable = disp_x
    boundary = left_left
    value = 0
  []
  [confine_right]
    type = DirichletBC
    variable = disp_x
    boundary = right_right
    value = 0
  []
  [push_down]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'left_top right_top'
    function = '-1e-3 * t'
  []
[]

[AuxVariables]
  [contact_force_y]
    block = '1 2 3'
  []
[]

[AuxKernels]
  [contact_force_y]
    type = TagVectorAux
    variable = contact_force_y
    v = disp_y
    vector_tag = contact_force
    block = '1 2 3'
  []
[]

[Postprocessors]
  [left_bottom_force]
    type = MortarContactForce
    weighted_gap_uo = lm_weightedgap_object_mortar_p_base_top_s_left_bottom
    component = y
  []
  [right_bottom_force]
    type = MortarContactForce
    weighted_gap_uo = lm_weightedgap_object_mortar_p_base_top_s_right_bottom
    component = y
  []
  [left_bottom_force_magnitude]
    type = MortarContactForce
    weighted_gap_uo = lm_weightedgap_object_mortar_p_base_top_s_left_bottom
  []
  [left_bottom_nodal_sum]
    type = NodalSum
    variable = contact_force_y
    boundary = left_bottom
  []
  [right_bottom_nodal_sum]
    type = NodalSum
    variable = contact_force_y
    boundary = right_bottom
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
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  dt = 1
  end_time = 2
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
[]
