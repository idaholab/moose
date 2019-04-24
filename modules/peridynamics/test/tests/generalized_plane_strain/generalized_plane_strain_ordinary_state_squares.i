[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[MeshGenerators]
  [fmg]
    type = FileMeshGenerator
    file = squares.e
  []
  [gpd]
    type = MeshGeneratorPD
    input = fmg
    retain_fe_mesh = false
  []
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3
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
    [./all]
      formulation = OrdinaryState
      block = '1 2'
    [../]
  [../]
  [./GeneralizedPlaneStrain]
    [./block1]
      formulation = OrdinaryState
      scalar_out_of_plane_strain = scalar_strain_zz1
      out_of_plane_stress_variable = stress_zz1
      block = 1
    [../]
    [./block2]
      formulation = OrdinaryState
      scalar_out_of_plane_strain = scalar_strain_zz2
      out_of_plane_stress_variable = stress_zz2
      block = 2
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
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_zz1
    scalar_out_of_plane_strain = scalar_strain_zz1

    poissons_ratio = 0.3
    youngs_modulus = 1e6
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5

    quantity_type = Component
    index_i = 2
    index_j = 2
    block = 1
  [../]

  [./stress_zz2]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_zz2
    scalar_out_of_plane_strain = scalar_strain_zz2

    poissons_ratio = 0.3
    youngs_modulus = 1e6
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5

    quantity_type = Component
    index_i = 2
    index_j = 2
    block = 2
  [../]
[]

[Postprocessors]
  [./react_z1]
    type = NodalVariableIntegralPD
    variable = stress_zz1
    block = 1
  [../]
  [./react_z2]
    type = NodalVariableIntegralPD
    variable = stress_zz2
    block = 2
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    value = '(1-x)*t'
  [../]
[]

[BCs]
  [./bottom1x]
    type = PresetBC
    boundary = 1
    variable = disp_x
    value = 0.0
  [../]
  [./bottom1y]
    type = PresetBC
    boundary = 1
    variable = disp_y
    value = 0.0
  [../]

  [./bottom2x]
    type = PresetBC
    boundary = 2
    variable = disp_x
    value = 0.0
  [../]
  [./bottom2y]
    type = PresetBC
    boundary = 2
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elastic_tensor1]
    type = SmallStrainVariableHorizonOSPD
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
    scalar_out_of_plane_strain = scalar_strain_zz1
    block = 1
  [../]
  [./elastic_tensor2]
    type = SmallStrainVariableHorizonOSPD
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
    scalar_out_of_plane_strain = scalar_strain_zz2
    block = 2
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
  file_base = generalized_plane_strain_ordinary_state_squares
[]
