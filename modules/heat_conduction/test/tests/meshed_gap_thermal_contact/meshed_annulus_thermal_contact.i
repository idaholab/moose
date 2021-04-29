[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = meshed_annulus.e
  []
  [rename]
    type = RenameBlockGenerator
    input = fmesh
    old_block = '1 2 3'
    new_block = '1 4 3'
  []
[]

[Variables]
  [./temp]
    block = '1 3'
    initial_condition = 1.0
  [../]
[]

[Kernels]
  [./hc]
    type = HeatConduction
    variable = temp
    block = '1 3'
  [../]
  [./source]
    type = HeatSource
    variable = temp
    block = 3
    value = 10.0
  [../]
[]

[BCs]
  [./outside]
    type = DirichletBC
    variable = temp
    boundary = 1
    value = 1.0
  [../]
[]

[ThermalContact]
  [./gap_conductivity]
    type = GapHeatTransfer
    variable = temp
    primary = 2
    secondary = 3
    emissivity_primary = 0
    emissivity_secondary = 0
    gap_conductivity = 0.5
  [../]
[]

[Materials]
  [./hcm]
    type = HeatConductionMaterial
    block = '1 3'
    temp = temp
    thermal_conductivity = 1
  [../]
[]

[Problem]
  type = FEProblem
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]
