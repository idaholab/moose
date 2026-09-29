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
    input = msh
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
    block_name = Block1
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = block1
    bottom_left = '0 1 0'
    top_right = '1 2 0'
    block_id = 2
    block_name = Block2
  []
  [split]
    type = BreakMeshByBlockGenerator
    input = block2
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        add_variables = true
        strain = SMALL
      []
    []
    [CohesiveZone]
      [czm_ik]
        boundary = 'Block1_Block2'
        strain = SMALL
      []
    []
  []
[]

[ICs]
  [disp_x_ic]
    type = ConstantIC
    variable = disp_x
    value = 0.005
    block = Block2
  []
  [disp_y_ic]
    type = ConstantIC
    variable = disp_y
    value = 0.015
    block = Block2
  []
[]

[BCs]
  [bottom_x]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
  []
  [top_x]
    type = DirichletBC
    boundary = top
    variable = disp_x
    value = 0.005
  []
  [top_y]
    type = DirichletBC
    boundary = top
    variable = disp_y
    value = 0.015
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [czm]
    type = BiLinearMixedModeTraction
    boundary = 'Block1_Block2'
    penalty_stiffness = 1e6
    GI_c = 1e3
    GII_c = 1e2
    normal_strength = 1e4
    shear_strength = 1e3
    eta = 2.2
    viscosity = 0.0
    lag_mode_mixity = false
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
  dt = 1
[]
