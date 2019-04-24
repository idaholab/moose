[GlobalParams]
  displacements = 'disp_x disp_y'
  temperature = temp

  poissons_ratio = 0.3
  youngs_modulus = 1e6
  thermal_expansion_coeff = 0.0002
  stress_free_temperature = 0.0
[]

[MeshGenerators]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
  []
  [gpd]
    type = MeshGeneratorPD
    input = gmg
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
[]

[AuxVariables]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./strain_xx]
    order = FIRST
    family = LAGRANGE
  [../]
  [./strain_yy]
    order = FIRST
    family = LAGRANGE
  [../]
  [./strain_zz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./strain_yz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./strain_xz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./strain_xy]
    order = FIRST
    family = LAGRANGE
  [../]

  [./stress_xx]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_yy]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_zz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_yz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_xz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_xy]
    order = FIRST
    family = LAGRANGE
  [../]

  [./von_mises]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = OrdinaryState
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]

  [./strain_xx]
    type = NodalStressStrainPD
    rank_two_tensor = total_strain
    variable = strain_xx
    quantity_type = Component
    index_i = 0
    index_j = 0
  [../]
  [./strain_yy]
    type = NodalStressStrainPD
    rank_two_tensor = total_strain
    variable = strain_yy
    quantity_type = Component
    index_i = 1
    index_j = 1
  [../]
  [./strain_zz]
    type = NodalStressStrainPD
    rank_two_tensor = total_strain
    variable = strain_zz
    quantity_type = Component
    index_i = 2
    index_j = 2
  [../]
  [./strain_yz]
    type = NodalStressStrainPD
    rank_two_tensor = total_strain
    variable = strain_yz
    quantity_type = Component
    index_i = 1
    index_j = 2
  [../]
  [./strain_xz]
    type = NodalStressStrainPD
    rank_two_tensor = total_strain
    variable = strain_xz
    quantity_type = Component
    index_i = 0
    index_j = 2
  [../]
  [./strain_xy]
    type = NodalStressStrainPD
    rank_two_tensor = total_strain
    variable = strain_xy
    quantity_type = Component
    index_i = 0
    index_j = 1
  [../]

  [./stress_xx]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_xx
    quantity_type = Component
    index_i = 0
    index_j = 0
  [../]
  [./stress_yy]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_yy
    quantity_type = Component
    index_i = 1
    index_j = 1
  [../]
  [./stress_zz]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_zz
    quantity_type = Component
    index_i = 2
    index_j = 2
  [../]
  [./stress_yz]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_yz
    quantity_type = Component
    index_i = 1
    index_j = 2
  [../]
  [./stress_xz]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_xz
    quantity_type = Component
    index_i = 0
    index_j = 2
  [../]
  [./stress_xy]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_xy
    quantity_type = Component
    index_i = 0
    index_j = 1
  [../]

  [./vonmises]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = von_mises
    quantity_type = VonMisesStress
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    value = 'x*x+y*y'
  [../]
[]

[BCs]
  [./leftx]
    type = PresetBC
    boundary = left
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elastic]
    type = SmallStrainConstantHorizonOSPD
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

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  file_base = nodal_stress_strain_aux
[]
