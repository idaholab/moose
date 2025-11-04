[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = false
[]

[Mesh]
  file = pressure.e
[]

[Problem]
  type = AugmentedLagrangianContactProblem
  maximum_lagrangian_update_iterations = 200
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_yy'
  []
[]

[Contact]
  [./m20_s10]
    primary = 20
    secondary = 10
    penalty = 1e7
    formulation = augmented_lagrange
    al_penetration_tolerance = 1e-8
    tangential_tolerance = 1e-3
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 3
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]

  [./z]
    type = DirichletBC
    variable = disp_z
    boundary = 5
    value = 0.0
  [../]

  [./Pressure]
    [./press]
      boundary = 7
      factor = 1e3
    [../]
  [../]

  [./down]
    type = DirichletBC
    variable = disp_y
    boundary = 8
    value = -2e-3
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff1_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Dampers]
  [./limitX]
    type = MaxIncrement
    max_increment = 1e-5
    variable = disp_x
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  #petsc_options_iname = '-pc_type -pc_hypre_type -snes_type -snes_ls -snes_linesearch_type -ksp_gmres_restart'
  #petsc_options_value = 'hypre    boomeramg      ls         basic    basic                    101'
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'

  line_search = 'none'

  nl_rel_tol = 1e-5
  nl_abs_tol = 1e-6

  l_tol = 1e-8

  l_max_its = 100
  nl_max_its = 20
  dt = 1.0
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
