[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./fmg]
    type = FileMeshGenerator
    file = squares.e
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = fmg
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./scalar_strain_zz1]
    order = FIRST
    family = SCALAR
  [../]
  [./scalar_strain_zz2]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./stress_zz1]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_zz2]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Modules/Peridynamics/Mechanics]
  [./Master]
    [./block1]
      formulation = ORDINARY_STATE
      block = 1001
    [../]
    [./block2]
      formulation = ORDINARY_STATE
      block = 1002
    [../]
  [../]
  [./GeneralizedPlaneStrain]
    [./block1]
      formulation = ORDINARY_STATE
      scalar_out_of_plane_strain = scalar_strain_zz1
      out_of_plane_stress_variable = stress_zz1
      block = 1001
    [../]
    [./block2]
      formulation = ORDINARY_STATE
      scalar_out_of_plane_strain = scalar_strain_zz2
      out_of_plane_stress_variable = stress_zz2
      block = 1002
    [../]
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]

  [./stress_zz1]
    type = NodalRankTwoPD
    variable = stress_zz1
    rank_two_tensor = stress
    scalar_out_of_plane_strain = scalar_strain_zz1

    poissons_ratio = 0.3
    youngs_modulus = 1e6
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5

    output_type = component
    index_i = 2
    index_j = 2
    block = 1001
  [../]

  [./stress_zz2]
    type = NodalRankTwoPD
    variable = stress_zz2
    scalar_out_of_plane_strain = scalar_strain_zz2

    poissons_ratio = 0.3
    youngs_modulus = 1e6
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5

    rank_two_tensor = stress
    output_type = component
    index_i = 2
    index_j = 2
    block = 1002
  [../]
[]

[Postprocessors]
  [./react_z1]
    type = NodalVariableIntegralPD
    variable = stress_zz1
    block = 1001
  [../]
  [./react_z2]
    type = NodalVariableIntegralPD
    variable = stress_zz2
    block = 1002
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    expression = '(1-x)*t'
  [../]
[]

[BCs]
  [./bottom1_x]
    type = DirichletBC
    boundary = 1001
    variable = disp_x
    value = 0.0
  [../]
  [./bottom1_y]
    type = DirichletBC
    boundary = 1001
    variable = disp_y
    value = 0.0
  [../]

  [./bottom2_x]
    type = DirichletBC
    boundary = 1002
    variable = disp_x
    value = 0.0
  [../]
  [./bottom2_y]
    type = DirichletBC
    boundary = 1002
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    block = '1001 1002'
  [../]

  [./force_density1]
    type = ComputeSmallStrainVariableHorizonMaterialOSPD
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
    scalar_out_of_plane_strain = scalar_strain_zz1
    block = 1001
  [../]
  [./force_density2]
    type = ComputeSmallStrainVariableHorizonMaterialOSPD
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
    scalar_out_of_plane_strain = scalar_strain_zz2
    block = 1002
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

  solve_type = PJFNK
  line_search = none

  l_tol = 1e-8
  nl_rel_tol = 1e-12

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  file_base = generalized_plane_strain_squares_OSPD
[]
