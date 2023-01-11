[Mesh]
  type = GeneratedMesh
  dim = 2
  # Mesh uses second-order elements
  elem_type = QUAD8
  displacements = 'disp_x disp_y'
  block_name = pore
  block_id = 0
[]

[Variables]
  [./temperature]
    order = SECOND
    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]
[]

# We are *not* allowed to use FIRST-order displacement vars!
[AuxVariables]
  [./disp_x]
  [../]

  [./disp_y]
    [./InitialCondition]
      type = FunctionIC
      function = displ
    [../]
  [../]
[]

[Functions]
  [./displ]
    type = ParsedFunction
    expression = -1/2*x*(y-0.5)
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = temperature
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 1
    use_displaced_mesh = true
  [../]
  [./right]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 0
    use_displaced_mesh = true
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = PJFNK
  [../]
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = none
  nl_rel_tol = 1e-6
  nl_max_its = 10
  l_tol = 1e-8
  l_max_its = 50
  num_steps = 2 # 200
  nl_abs_tol = 1e-10
  nl_rel_step_tol = 1e-10
  nl_abs_step_tol = 1e-10
  [./TimeStepper]
    type = ConstantDT
    dt = 0.001
  [../]
  dtmin = .001
[]

