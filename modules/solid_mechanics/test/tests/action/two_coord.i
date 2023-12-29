[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 8
    xmin = -1
    xmax = 1
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '-1 0 0'
    top_right = '0 1 0'
    input = generated_mesh
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    input = block1
  []
[]

[Problem]
  coord_type = 'XYZ RZ'
  block      = '1   2'
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules/TensorMechanics/Master]
  active = 'block1 block2'
  [./error]
    strain = SMALL
    add_variables = true
  [../]
  [./block1]
    strain = SMALL
    add_variables = true
    block = 1
  [../]
  [./block2]
    strain = SMALL
    add_variables = true
    block = 2
  [../]
[]

[AuxVariables]
  [./vmstress]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./vmstress]
    type = RankTwoScalarAux
    rank_two_tensor = total_strain
    variable = vmstress
    scalar_type = VonMisesStress
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.345
  [../]
  [./_elastic_stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  [../]
[]

[BCs]
  [./topx]
    type = DirichletBC
    boundary = 'top'
    variable = disp_x
    value = 0.0
  [../]
  [./topy]
    type = DirichletBC
    boundary = 'top'
    variable = disp_y
    value = 0.0
  [../]
  [./bottomx]
    type = DirichletBC
    boundary = 'bottom'
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = DirichletBC
    boundary = 'bottom'
    variable = disp_y
    value = 0.05
  [../]
[]

[Debug]
  show_var_residual_norms = true
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
