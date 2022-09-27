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
    block_name = 'block1'
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = 'block1'
    bottom_left = '0 1 0'
    top_right = '1 2 0'
    block_id = 2
    block_name = 'block2'
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

[Modules]
  [TensorMechanics]
    [Master]
      generate_output = 'stress_yy'
      [all]
        strain = FINITE
        add_variables = true
        use_automatic_differentiation = true
        decomposition_method = TaylorExpansion
      []
    []
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

[Modules/TensorMechanics/CohesiveZoneMaster]
  [czm_ik]
    boundary = 'interface'
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
    boundary = 'interface'
    penalty_stiffness = 1e6
    GI_c = 1e3
    GII_c = 1e2
    normal_strength = 1e4
    shear_strength = 1e3
    displacements = 'disp_x disp_y'
    eta = 2.2
    viscosity = 1e-3
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
  dt = 0.1
  end_time = 1.0
  dtmin = 0.1
[]

[Outputs]
  exodus = true
[]
