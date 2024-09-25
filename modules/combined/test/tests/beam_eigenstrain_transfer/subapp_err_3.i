# SubApp with 2D model to test multi app vectorpostprocessor to aux var transfer

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 5
  xmin = 0.0
  xmax = 0.5
  ymin = 0.0
  ymax = 0.150080
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./temp]
  [../]
  [./axial_strain]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[Functions]
  [./temperature_load]
    type = ParsedFunction
    expression = t*(500.0)+300.0
  [../]
[]

[Physics]
  [./SolidMechanics]
    [./QuasiStatic]
      [./all]
        strain = SMALL
        incremental = true
        add_variables = true
        eigenstrain_names = eigenstrain
      [../]
    [../]
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = temperature_load
  [../]
  [./axial_strain]
    type = RankTwoAux
    variable = axial_strain
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./small_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./thermal_expansion_strain]
    type = ComputeThermalExpansionEigenstrain
    stress_free_temperature = 298
    thermal_expansion_coeff = 1.3e-5
    temperature = temp
    eigenstrain_name = eigenstrain
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 50
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.075
  dt = 0.0125
  dtmin = 0.0001
[]

[Outputs]
  exodus = true
[]

[VectorPostprocessors]
  [./axial_str]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    start_point = '0.5 0.0 0.0'
    end_point = '0.5 0.150080 0.0'
    variable = axial_strain
    num_points = 21
    sort_by = 'y'
  [../]
[]

[Postprocessors]
  [./end_disp]
    type = PointValue
    variable = disp_y
    point = '0.5 0.150080 0.0'
  [../]
[]
