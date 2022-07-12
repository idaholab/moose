[GlobalParams]
  displacements = 'disp_r disp_z'
  order = SECOND
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  elem_type = QUAD9
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [./disp_r]
  [../]
  [./disp_z]
  [../]
  [./u]
    order = FIRST
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./disp_r]
    type = Diffusion
    variable = disp_r
  [../]
  [./disp_z]
    type = Diffusion
    variable = disp_z
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
  [./disp_z_left]
    type = DirichletBC
    preset = false
    value = 0
    boundary = 'bottom'
    variable = disp_z
  [../]
  [./disp_z_right]
    type = DirichletBC
    preset = false
    value = 1
    boundary = 'top'
    variable = disp_z
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
  [./disp_z]
    type = RandomIC
    variable = disp_z
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
