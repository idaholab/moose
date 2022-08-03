# ---------------------------------------------------------------------------------------------------------
# REGRESSION TEST FOR OVERCLOSURE REMOVAL
# =======================================
# THIS TEST DEMONSTRATES THAT THE CODE IS CAPABLE OF REMOVING A SIZEABLE OVERCLOSURE IN A SINGLE TIME-STEP
# --------------------------------------------------------------------------------------------------------


[Mesh]
  file = oc_mesh.e
[]

[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
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
[]

[AuxVariables]
  [./penetration]
    order = FIRST
    family = LAGRANGE
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
    use_finite_deform_jacobian = true
  [../]
[]

[AuxKernels]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 4
    paired_boundary = 3
  [../]
[]

[Postprocessors]
  [./bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 1
  [../]
  [./bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 1
  [../]
  [./top_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 5
  [../]
  [./top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 5
  [../]
  [./ref_resid_x]
    type = NodalL2Norm
    execute_on = timestep_end
    variable = saved_x
  [../]
  [./ref_resid_y]
    type = NodalL2Norm
    execute_on = timestep_end
    variable = saved_y
  [../]
  [./_dt]
    type = TimestepSize
  [../]
[]

[BCs]
  [./bot_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./side1_x]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0.0
  [../]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = 5
    value = 0.0
  [../]
  [./top_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1001    #nodeset 1001 top central node
    value = 0.0
  [../]
[]

[Materials]
  [./bot_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./bot_strain]
    type = ComputeFiniteStrain
    decomposition_method = EigenSolution
    block = '1'
  [../]
  [./bot_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  [../]

  [./top_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1e5
    poissons_ratio = 0.3
  [../]
  [./top_strain]
    type = ComputeFiniteStrain
    decomposition_method = EigenSolution
    block = '2'
  [../]
  [./top_stress]
    type = ComputeFiniteStrainElasticStress
    block = '2'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-9
  l_max_its = 100
  nl_max_its = 200
  dt = 1.0
  end_time = 1.0
  dtmin = 1.0
  l_tol = 1e-3

[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
[]

[Contact]
  [./leftright]
    secondary = 4
    primary = 3
    model = frictionless
    formulation = penalty
    normalize_penalty = true
    tangential_tolerance = 1e-3
    penalty = 1e+9
  [../]
[]
