[GlobalParams]
  displacements = 'disp_r'
  order = SECOND
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  elem_type = EDGE3
[]

[Problem]
  coord_type = RSPHERICAL
[]

[Variables]
  [./disp_r]
  [../]
  [./u]
    order = FIRST
  [../]
[]

[Kernels]
  [./disp_r]
    type = Diffusion
    variable = disp_r
  [../]
  [./u]
    type = ADDiffusion
    variable = u
    use_displaced_mesh = true
  [../]
[]

[BCs]
  # BCs cannot be preset due to Jacobian tests
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
  [./disp_r_left]
    type = DirichletBC
    preset = false
    value = 0
    boundary = 'left'
    variable = disp_r
  [../]
  [./disp_r_right]
    type = DirichletBC
    preset = false
    value = 1
    boundary = 'right'
    variable = disp_r
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
  [./disp_r]
    type = RandomIC
    variable = disp_r
    min = 0.01
    max = 0.09
  [../]
  [./u]
    type = RandomIC
    variable = u
    min = 0.1
    max = 0.9
  [../]
[]
