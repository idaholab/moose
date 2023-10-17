[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2blk-gap.e
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '101'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
    input = file
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '100'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
    input = secondary
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Variables]
  [./temp]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  [../]

  [./lm]
    order = FIRST
    family = LAGRANGE
    block = 'secondary_lower'
  [../]
[]

[Materials]
  [./left]
    type = HeatConductionMaterial
    block = 1
    thermal_conductivity = 1000
    specific_heat = 1
  [../]

  [./right]
    type = HeatConductionMaterial
    block = 2
    thermal_conductivity = 500
    specific_heat = 1
  [../]
[]

[Kernels]
  [./hc]
    type = HeatConduction
    variable = temp
    use_displaced_mesh = false
    block = '1 2'
  [../]
[]

[Constraints]
  [./ced]
    type = GapConductanceConstraint
    variable = lm
    secondary_variable = temp
    k = 100
    primary_boundary = 100
    primary_subdomain = 10000
    secondary_boundary = 101
    secondary_subdomain = 10001
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = temp
    boundary = 'left'
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = temp
    boundary = 'right'
    value = 0
  [../]
[]

[Preconditioning]
  [./fmp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-11
[]

[Outputs]
  exodus = true
[]
