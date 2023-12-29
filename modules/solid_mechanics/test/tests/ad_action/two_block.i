[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
    input = generated_mesh
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    input = block1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules/TensorMechanics/Master]
  [./block1]
    strain = FINITE
    add_variables = true
    #block = 1
    use_automatic_differentiation = true
  [../]
  [./block2]
    strain = SMALL
    add_variables = true
    block = 2
    use_automatic_differentiation = true
  [../]
[]

[AuxVariables]
  [./stress_theta]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_theta]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_theta]
    type = ADRankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = stress_theta
    execute_on = timestep_end
  [../]
  [./strain_theta]
    type = ADRankTwoAux
    rank_two_tensor = total_strain
    index_i = 2
    index_j = 2
    variable = strain_theta
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.345
  [../]
  [./_elastic_stress1]
    type = ADComputeFiniteStrainElasticStress
    block = 1
  [../]
  [./_elastic_stress2]
    type = ADComputeLinearElasticStress
    block = 2
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    boundary = 'left'
    variable = disp_x
    value = 0.0
  [../]
  [./top]
    type = DirichletBC
    boundary = 'top'
    variable = disp_y
    value = 0.0
  [../]
  [./right]
    type = DirichletBC
    boundary = 'right'
    variable = disp_x
    value = 0.01
  [../]
  [./bottom]
    type = DirichletBC
    boundary = 'bottom'
    variable = disp_y
    value = 0.01
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]

[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '  201               hypre    boomeramg      10'

  line_search = 'none'

  nl_rel_tol = 5e-9
  nl_abs_tol = 1e-10
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 50
[]

[Outputs]
  exodus = true
[]
