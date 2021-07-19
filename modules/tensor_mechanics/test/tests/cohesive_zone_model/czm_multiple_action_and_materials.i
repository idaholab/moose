[Mesh]
  [./msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 4
    zmin = 0
    zmax = 4
  [../]
  [./subdomain_id]
    type = ElementSubdomainIDGenerator
    input = msh
    subdomain_ids = '0 1 2 3'
  []
  [./split]
    type = BreakMeshByBlockGenerator
    input = subdomain_id
    split_interface = true
  []
  [add_side_sets]
    input = split
    type = SideSetsFromNormalsGenerator
    normals = '0 -1  0
               0  1  0
               -1 0  0
               1  0  0
               0  0 -1
               0  0  1'
    fixed_normal = true
    new_boundary = 'y0 y1 x0 x1 z0 z1'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./stretch]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 100'
  [../]
[]

[Constraints]
  [x1]
    type = EqualValueBoundaryConstraint
    variable = disp_x
    secondary = 'x1'    # boundary
    penalty = 1e6
  []
  [y1]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    secondary = 'y1'    # boundary
    penalty = 1e6
  []
[]

[BCs]
  [./fix_x]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = x0
    variable = disp_x
  [../]
  [./fix_y]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = y0
    variable = disp_y
  [../]
  [./fix_z]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = z0
    variable = disp_z
  [../]
  [./back_z]
    type = FunctionNeumannBC
    boundary = z1
    variable = disp_z
    use_displaced_mesh = false
    function = stretch
  [../]
[]


[Modules/TensorMechanics/CohesiveZoneMaster]
  [./czm_ik_012]
    boundary = 'Block0_Block1 Block1_Block2'
    base_name = 'czm_b012'
  [../]
  [./czm_ik_23]
    boundary = 'Block2_Block3'
    base_name = 'czm_b23'
  [../]
[]

[Materials]
  # cohesive materials
  [./czm_3dc]
    type = SalehaniIrani3DCTraction
    boundary = 'Block0_Block1 Block1_Block2'
    normal_gap_at_maximum_normal_traction = 1
    tangential_gap_at_maximum_shear_traction = 0.5
    maximum_normal_traction = 500
    maximum_shear_traction = 300
    base_name = 'czm_b012'
  [../]
  [./czm_elastic_incremental]
    type = PureElasticTractionSeparationIncremental
    boundary = 'Block2_Block3'
    normal_stiffness = 500
    tangent_stiffness = 300
    base_name = 'czm_b23'
  [../]
  # bulk materials
  [./stress]
    type = ADComputeFiniteStrainElasticStress
  [../]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200e4
    poissons_ratio = 0.3
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        strain = FINITE
        add_variables = true
        use_finite_deform_jacobian = true
        use_automatic_differentiation = true
        generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_xz'
      [../]
    [../]
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  # Executioner
  type = Transient

  solve_type = 'NEWTON'
  line_search = none
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-6
  l_max_its = 20
  start_time = 0.0
  dt = 0.25
  dtmin = 0.25
  num_steps =1
[]

[Outputs]
  exodus = true
[]
