[Mesh]
  type = FileMesh
  file = square.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_cond_func
    [../]
  [../]
[]

[AuxVariables]
  [./u_aux]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = initial_cond_func
    [../]
  [../]
[]

[Functions]
  [./initial_cond_func]
    type = SolutionFunction
    solution = ex_soln
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[UserObjects]
  [./ex_soln]
    type = SolutionUserObject
    variables = u_aux
    system = AuxiliarySystem
    mesh = out_0001_mesh.xda
    es = out_0001.xda
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = -snes_mf_operator
  nl_rel_tol = 1e-10
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
