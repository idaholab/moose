[Mesh]
  file = nodal_normals_test_offset_nonmatching_gap.e
  # block 1: left
  # block 2: right

  displacements = 'disp_x disp_y'
[]

[MeshModifiers]
  [./master]
    type = LowerDBlockFromSideset
    sidesets = '2'
    new_block_id = '20'
  [../]
  [./slave]
    type = LowerDBlockFromSideset
    sidesets = '1'
    new_block_id = '10'
  [../]
[]

[AuxVariables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
[]

[AuxKernels]
  [function_x]
    type = FunctionAux
    function = '.05 * t'
    variable = 'disp_x'
    block = '2'
  []
  [function_y]
    type = FunctionAux
    function = '.05 * t'
    variable = 'disp_y'
    block = '2'
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./T]
    block = '1 2'
  [../]
  [./lambda]
    block = '10'
  [../]
[]

[BCs]
  [./vacuum]
    type = VacuumBC
    variable = T
    boundary = '3 4 5 6 7 8'
  [../]
[]

[Kernels]
  [./conduction]
    type = Diffusion
    variable = T
    block = '1 2'
  [../]
  [./uniform_source]
    type = BodyForce
    variable = T
    block = '1 2'
  [../]
[]

[Debug]
  show_var_residual_norms = 1
[]

[Constraints]
  [./mortar]
    type = GapHeatConductanceTest
    master_boundary_id = 2
    slave_boundary_id = 1
    master_subdomain_id = 20
    slave_subdomain_id = 10
    variable = lambda
    master_variable = T
    use_displaced_mesh = true
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  num_steps = 5
  petsc_options_iname = '-pc_type -snes_linesearch_type'
  petsc_options_value = 'lu       basic'
[]

[Outputs]
  exodus = true
  [dofmap]
    type = DOFMap
    execute_on = 'initial'
  []
[]
