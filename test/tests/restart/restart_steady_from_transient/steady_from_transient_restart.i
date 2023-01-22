[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = transient_out_cp/LATEST
  []
  parallel_type = replicated
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = ((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = -4
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]

[Problem]
  restart_file_base = transient_out_cp/LATEST
[]
