[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  displacements = 'disp_x disp_y disp_z'
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./total_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./top_pull]
    type = PiecewiseLinear
    x = '0 1     2'
    y = '0 0.025 0.05'
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]

  [./total_strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = total_strain_yy
    index_i = 1
    index_j = 1
  [../]
[]


[BCs]
  [./y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 3
    function = top_pull
  [../]

  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]

  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]

  [./z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = 0
    value = 0.0
  [../]
[]

[Postprocessors]
  [./stress_yy_el]
    type = ElementalVariableValue
    variable = stress_yy
    elementid = 0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 2e5
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Dampers]
  [./disp_x_damp]
    type = ElementJacobianDamper
    max_increment = 0.002
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time = 2
  dt = 1
[]

[Outputs]
  exodus = true
[]
