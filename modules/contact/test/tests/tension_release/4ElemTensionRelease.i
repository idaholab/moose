[Mesh]
  file = 4ElemTensionRelease.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[Functions]
  [./up]
    type = PiecewiseLinear
    x = '0 1      2 3'
    y = '0 0.0001 0 -.0001'
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    strain = FINITE
  []
[]

[Contact]
  [./dummy_name]
    primary = 2
    secondary = 3
    penalty = 1e6
    model = frictionless
    tangential_tolerance = 0.01
  [../]
[]

[BCs]
  [./lateral]
    type = DirichletBC
    variable = disp_x
    boundary = '1 4'
    value = 0
  [../]

  [./bottom_up]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1
    function = up
  [../]

  [./top]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  [../]
  [./stiffStuff1_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      101'

  line_search = 'none'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  l_tol = 1e-4

  l_max_its = 100
  nl_max_its = 10
  dt = 0.2
  dtmin = 0.2
  end_time = 3

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]
[]

[Outputs]
  exodus = true
[]
