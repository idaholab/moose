boundary = 'Block1_Block2'

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 2
    nx = 1
    ny = 2
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = 'msh'
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
    block_name = 'Block1'
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = 'block1'
    bottom_left = '0 1 0'
    top_right = '1 2 0'
    block_id = 2
    block_name = 'Block2'
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = block2
  []
  [top_node]
    type = ExtraNodesetGenerator
    coord = '0 2 0'
    input = split
    new_boundary = top_node
  []
  [bottom_node]
    type = ExtraNodesetGenerator
    coord = '0 0 0'
    input = top_node
    new_boundary = bottom_node
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      generate_output = 'stress_yy'
      [all]
        strain = FINITE
        add_variables = true
        use_automatic_differentiation = true
        decomposition_method = TaylorExpansion
        extra_vector_tags = 'ref'
      []
    []
  []
[]

[AuxVariables]
  [react_y]
  []
[]

[AuxKernels]
  [react_y]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_y'
    variable = 'react_y'
    remove_variable_scaling = true
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = bottom_node
    variable = disp_x
  []

  [fix_top]
    type = DirichletBC
    preset = true
    boundary = top
    variable = disp_x
    value = 0
  []

  [top]
    type = FunctionDirichletBC
    boundary = top
    variable = disp_y
    function = 'if(t<=0.3,t,if(t<=0.6,0.3-(t-0.3),0.6-t))'
    preset = true
  []

  [bottom]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
    preset = true
  []
[]

[Physics/SolidMechanics/CohesiveZone]
  [czm_ik]
    boundary = ${boundary}
  []
[]

[Materials]
  [stress]
    type = ADComputeFiniteStrainElasticStress
  []
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1.684e5 0.176e5 0.176e5 1.684e5 0.176e5 1.684e5 0.754e5 0.754e5 0.754e5'
  []
  [czm]
    type = BiLinearMixedModeTraction
    boundary = ${boundary}
    penalty_stiffness = 1e6
    GI_c = 1e3
    GII_c = 1e2
    normal_strength = 1e4
    shear_strength = 1e3
    displacements = 'disp_x disp_y'
    eta = 2.2
    # viscosity=1e-3 + lag_mode_mixity=true not present on the NEML2 side;
    # disable both so this MOOSE reference matches the NEML2 TSL model here
    viscosity = 0
    lag_mode_mixity = false
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

  solve_type = 'NEWTON'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 30
  nl_rel_tol = 1e-50
  nl_abs_tol = 1e-15
  start_time = 0.0
  end_time = 1.0

  # Small time steps at the start resolve damage nucleation (the interface jumps past the
  # onset separation delta_c = normal_strength / penalty_stiffness = 0.01 in one step
  # otherwise), and again through the peak-to-failure window; coarse steps suffice elsewhere.
  [TimeStepper]
    type = TimeSequenceStepper
    time_sequence = '0 0.001 0.002 0.005 0.01 0.03 0.06 0.08 0.1 0.12 0.14 0.16 0.18 0.2 0.25 0.3 0.4 0.5 0.52 0.54 0.56 0.58 0.6 0.62 0.64 0.66 0.68 0.7 0.72 0.74 0.76 0.78 0.8 0.82 0.84 0.86 0.88 0.9 0.92 0.94 0.96 0.98 1.0'
  []
[]

[Postprocessors]
  [react_y]
    type = NodalSum
    variable = 'react_y'
    boundary = 'top'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]