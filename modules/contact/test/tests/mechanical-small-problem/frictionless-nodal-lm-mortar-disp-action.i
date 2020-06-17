[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [./simple_mesh]
    type = FileMeshGenerator
    file = mesh.e
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
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'outside_right'
    function = '-5e-3 * t'
  [../]
  [./right_y]
    type = FunctionDirichletBC
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

[Contact]
  [frictionless]
    mesh = simple_mesh
    master = 2
    secondary = 1
    formulation = mortar
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
  hide = 'contact_pressure nodal_area_frictionless penetration'
[]

[Postprocessors]
  [contact]
    type = ContactDOFSetSize
    variable = frictionless_normal_lm
    subdomain = '4'
    execute_on = 'nonlinear timestep_end'
  []
[]

