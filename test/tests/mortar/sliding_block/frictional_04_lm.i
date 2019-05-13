#  This is a benchmark test that checks constraint based frictional
#  contact using the penalty method.  In this test a sinusoidal
#  displacement is applied in the horizontal direction to simulate
#  a small block come in and out of contact as it slides down a larger block.
#
#  The sinusoid is of the form 0.4sin(4t)+0.2 and a friction coefficient
#  of 0.4 is used.  The gold file is run on one processor and the benchmark
#  case is run on a minimum of 4 processors to ensure no parallel variability
#  in the contact pressure and penetration results.  Further documentation can
#  found in moose/modules/contact/doc/sliding_block/
#

[Mesh]
  file = sliding_elastic_blocks_2d.e
[]

[MeshModifiers]
  [slave]
    type = LowerDBlockFromSideset
    sidesets = '3'
    new_block_id = '30'
  []
  [master]
    type = LowerDBlockFromSideset
    sidesets = '2'
    new_block_id = '20'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
    block = '1 2'
  [../]
  [./disp_y]
    block = '1 2'
  [../]
  [normal_lm]
    block = '30'
    family = MONOMIAL
    order = CONSTANT
  []
  [tangential_lm]
    block = '30'
    family = MONOMIAL
    order = CONSTANT
  []
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./right_x]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 4
    function = horizontal_movement
  [../]
  [./right_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 4
    function = vertical_movement
  [../]
[]

[Kernels]
  [diff_x]
    type = Diffusion
    block = '1 2'
    variable = disp_x
  []
  [diff_y]
    type = Diffusion
    block = '1 2'
    variable = disp_y
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  num_steps = 30
  dtmin = 1
[]

[Outputs]
  exodus = true
  [dof]
    execute_on = 'initial'
    type = DOFMap
  []
[]

[Functions]
  [./vertical_movement]
    type = ParsedFunction
    value = -t
  [../]
  [./horizontal_movement]
    type = ParsedFunction
    value = -0.04*sin(4*t)+0.02
  [../]
[]

[Constraints]
  [normal_lm]
    type = MechanicalContactLMTest
    master_boundary = '2'
    slave_boundary = '1'
    master_subdomain = '20'
    slave_subdomain = '10'
    variable = normal_lm
    slave_variable = disp_x
    slave_disp_y = disp_y
    use_displaced_mesh = true
    compute_primal_residuals = false
  []
  [normal_x]
    type = MechanicalContactTest
    master_boundary = '2'
    slave_boundary = '1'
    master_subdomain = '20'
    slave_subdomain = '10'
    variable = normal_lm
    slave_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
  [normal_y]
    type = MechanicalContactTest
    master_boundary = '2'
    slave_boundary = '1'
    master_subdomain = '20'
    slave_subdomain = '10'
    variable = normal_lm
    slave_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
  [tangential_lm]
    type = TangentialContactLMTest
    master_boundary = '2'
    slave_boundary = '1'
    master_subdomain = '20'
    slave_subdomain = '10'
    variable = tangential_lm
    slave_variable = disp_x
    slave_disp_y = disp_y
    use_displaced_mesh = true
    compute_primal_residuals = false
    contact_pressure = normal_lm
    friction_coefficient = .4
  []
  [tangential_x]
    type = TangentialContactTest
    master_boundary = '2'
    slave_boundary = '1'
    master_subdomain = '20'
    slave_subdomain = '10'
    variable = tangential_lm
    slave_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
  [tangential_y]
    type = TangentialContactTest
    master_boundary = '2'
    slave_boundary = '1'
    master_subdomain = '20'
    slave_subdomain = '10'
    variable = tangential_lm
    slave_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
  []
[]
