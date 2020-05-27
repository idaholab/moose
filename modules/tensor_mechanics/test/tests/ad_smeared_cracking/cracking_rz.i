#

[Mesh]
  file = cracking_rz_test.e
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Functions]
  [./displ]
    type = PiecewiseLinear
    x = '0 1 2 3  4'
    y = '0 1 0 -1 0'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
    use_automatic_differentiation = true
  [../]
[]

[BCs]
  [./pull]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = 2
    function = displ
  [../]
  [./left]
    type = ADDirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./bottom]
    type = ADDirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 4.0e7
    poissons_ratio = 0.0
  [../]
  [./elastic_stress]
    type = ADComputeSmearedCrackingStress
    cracking_stress = 1.68e6
    softening_models = abrupt_softening
  [../]
  [./abrupt_softening]
    type = ADAbruptSoftening
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton

  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101               '

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-2
  l_tol = 1e-5
  start_time = 0.0
  end_time = 0.1
  dt = 0.025
[]

[Outputs]
  exodus = true
[]
