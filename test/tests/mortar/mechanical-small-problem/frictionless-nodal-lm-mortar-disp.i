[Mesh]
  file = mesh.e
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

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./disp_x]
    block = '1 2'
  [../]
  [./disp_y]
    block = '1 2'
  [../]
  [./lambda]
    block = '10'
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'outside_left'
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'outside_left'
    value = 0.0
  [../]
  [./right_x]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 'outside_right'
    function = '-5e-3 * t'
  [../]
  [./right_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 'outside_right'
    function = 0
  [../]
[]

[Kernels]
  [disp_x]
    type = Diffusion
    variable = disp_x
    block = '1 2'
  []
  [disp_y]
    type = Diffusion
    variable = disp_y
    block = '1 2'
  []
[]

[Debug]
  show_var_residual_norms = 1
[]

[Constraints]
  [./lm]
    type = NormalNodalLMMechanicalContact
    slave = 1
    master = 2
    variable = lambda
    master_variable = disp_x
    disp_y = disp_y
  [../]
  [x]
    type = NormalMortarMechanicalContact
    master_boundary = '2'
    slave_boundary = '1'
    master_subdomain = '20'
    slave_subdomain = '10'
    variable = lambda
    slave_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
  [y]
    type = NormalMortarMechanicalContact
    master_boundary = '2'
    slave_boundary = '1'
    master_subdomain = '20'
    slave_subdomain = '10'
    variable = lambda
    slave_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
[]


[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  solve_type = PJFNK
  type = Transient
  num_steps = 10
  dt = 1
  dtmin = 1
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       basic                 NONZERO               1e-15'
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [contact]
    type = ContactDofSet
    variable = lambda
    subdomain = '10'
    execute_on = 'nonlinear timestep_end'
  []
[]
