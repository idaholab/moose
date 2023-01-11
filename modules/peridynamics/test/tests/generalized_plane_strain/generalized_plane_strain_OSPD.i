[GlobalParams]
  displacements = 'disp_x disp_y'
  scalar_out_of_plane_strain = scalar_strain_zz
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./scalar_strain_zz]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./stress_zz]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Modules/Peridynamics/Mechanics]
  [./Master]
    [./all]
      formulation = ORDINARY_STATE
    [../]
  [../]
  [./GeneralizedPlaneStrain]
    [./all]
      formulation = ORDINARY_STATE
      out_of_plane_stress_variable = stress_zz
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

  [./stress_zz]
    type = NodalRankTwoPD
    variable = stress_zz

    poissons_ratio = 0.3
    youngs_modulus = 1e6
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5

    rank_two_tensor = stress
    output_type = component
    index_i = 2
    index_j = 2
  [../]
[]

[Postprocessors]
  [./react_z]
    type = NodalVariableIntegralPD
    variable = stress_zz
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    expression = '(1-x)*t'
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    boundary = 1000
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = 1000
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]

  [./force_density]
    type = ComputeSmallStrainVariableHorizonMaterialOSPD
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
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

  nl_rel_tol = 1e-12

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  file_base = generalized_plane_strain_OSPD
[]
