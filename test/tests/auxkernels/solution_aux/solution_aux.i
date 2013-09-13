[Mesh]
  type = FileMesh
  file = square.e
  # This test uses SolutionUserObject which doesn't work with ParallelMesh.
  distribution = serial
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
    solution = xda_soln
    execute_on = initial
    variable = u_aux
  [../]
[]

[UserObjects]
  [./xda_soln]
    type = SolutionUserObject
    mesh = out_0001_mesh.xda
    es = out_0001.xda
    system = AuxiliarySystem
    variables = u_aux
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

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_rel_tol = 1e-10
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]
