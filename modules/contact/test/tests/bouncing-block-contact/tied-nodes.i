[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1e0
[]

[Mesh]
  file = long-bottom-block-symmetric-single-element.e
[]

[Variables]
  [./disp_x]
    scaling = 2
  [../]
  [./disp_y]
    scaling = 3
  [../]
[]

[Kernels]
  [./disp_x]
    type = MatDiffusion
    variable = disp_x
  [../]
  [./disp_y]
    type = MatDiffusion
    variable = disp_y
  [../]
[]


[Constraints]
  [./disp_x]
    type = RANFSTieNode
    secondary = 10
    primary = 20
    variable = disp_x
    primary_variable = disp_x
    component = x
  [../]
  [./disp_y]
    type = RANFSTieNode
    secondary = 10
    primary = 20
    variable = disp_y
    primary_variable = disp_y
    component = y
  [../]
[]

[BCs]
  [./botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  [../]
  [./boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  [../]
  [./topy]
    type = DirichletBC
    variable = disp_y
    boundary = 30
    value = 0
  [../]
  [./topx]
    type = DirichletBC
    variable = disp_x
    boundary = 30
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dtmin = 1
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
  l_max_its = 30
  nl_max_its = 20
  line_search = 'none'
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  [exo]
    type = Exodus
  []
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [./num_nl]
    type = NumNonlinearIterations
  [../]
  [./cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  [../]
[]
