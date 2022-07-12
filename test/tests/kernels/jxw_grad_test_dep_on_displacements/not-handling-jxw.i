[GlobalParams]
  displacements = 'disp_x disp_y'
  order = SECOND
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./u]
    order = FIRST
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./disp_x]
    type = Diffusion
    variable = disp_x
  [../]
  [./disp_y]
    type = Diffusion
    variable = disp_y
  [../]
  [./u]
    type = ADDiffusion
    variable = u
    use_displaced_mesh = true
  [../]
  [./v]
    type = ADDiffusion
    variable = v
    use_displaced_mesh = true
  [../]
[]

[BCs]
  # BCs cannot be preset due to Jacobian test
  [./u_left]
    type = DirichletBC
    preset = false
    value = 0
    boundary = 'left'
    variable = u
  [../]
  [./u_right]
    type = DirichletBC
    preset = false
    value = 1
    boundary = 'right'
    variable = u
  [../]
  [./v_left]
    type = DirichletBC
    preset = false
    value = 0
    boundary = 'left'
    variable = v
  [../]
  [./v_right]
    type = DirichletBC
    preset = false
    value = 1
    boundary = 'right'
    variable = v
  [../]
  [./disp_x_left]
    type = DirichletBC
    preset = false
    value = 0
    boundary = 'left'
    variable = disp_x
  [../]
  [./disp_x_right]
    type = DirichletBC
    preset = false
    value = 1
    boundary = 'right'
    variable = disp_x
  [../]
  [./disp_y_left]
    type = DirichletBC
    preset = false
    value = 0
    boundary = 'bottom'
    variable = disp_y
  [../]
  [./disp_y_right]
    type = DirichletBC
    preset = false
    value = 1
    boundary = 'top'
    variable = disp_y
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  [./dofmap]
    type = DOFMap
    execute_on = 'initial'
  [../]
[]

[ICs]
  [./disp_x]
    type = RandomIC
    variable = disp_x
    min = 0.01
    max = 0.09
  [../]
  [./disp_y]
    type = RandomIC
    variable = disp_y
    min = 0.01
    max = 0.09
  [../]
  [./u]
    type = RandomIC
    variable = u
    min = 0.1
    max = 0.9
  [../]
  [./v]
    type = RandomIC
    variable = v
    min = 0.1
    max = 0.9
  [../]
[]
