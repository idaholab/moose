[Mesh]
  type = FileMesh
  file = square.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./u_aux]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./initial_cond_aux]
    type = SolutionAux
    mesh = out_0001_mesh.xda
    es = out_0001.xda
    system = AuxiliarySystem
    variable = u_aux
    execute_on = initial
    from_variable = u_aux
    direct = true
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

[Executioner]
  type = Steady
  petsc_options = -snes_mf_operator
  nl_rel_tol = 1e-10
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

