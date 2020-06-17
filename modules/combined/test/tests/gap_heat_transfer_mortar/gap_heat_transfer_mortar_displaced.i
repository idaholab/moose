[Mesh]
  displacements = 'disp_x disp_y'
  [file]
    type = FileMeshGenerator
    file = 2blk-gap.e
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '101'
    new_block_id = 10001
    new_block_name = 'secondary_lower'
    input = file
  []
  [master]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '100'
    new_block_id = 10000
    new_block_name = 'master_lower'
    input = secondary
  []
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[AuxVariables]
  [./disp_x]
    block = '1'
  [../]
  [./disp_y]
    block = '1'
  [../]
[]

[AuxKernels]
  [./disp_x_kernel]
    type = ConstantAux
    variable = disp_x
    value = 0.1
    block = '1'
  [../]

  [./disp_y_kernel]
    type = ConstantAux
    variable = disp_y
    value = 0
    block = '1'
  [../]
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
  [./hc_displaced_block]
    type = HeatConduction
    variable = temp
    use_displaced_mesh = true
    block = '1'
  [../]
  [./hc_undisplaced_block]
    type = HeatConduction
    variable = temp
    use_displaced_mesh = false
    block = '2'
  [../]
[]

[Constraints]
  [./ced]
    type = GapConductanceConstraint
    variable = lm
    secondary_variable = temp
    k = 100
    use_displaced_mesh = true
    master_boundary = 100
    master_subdomain = 10000
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
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-11
[]

[Outputs]
  exodus = true
  show = 'temp disp_x disp_y'
[]
