[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./err]
    order = FIRST
    family = LAGRANGE
  [../]
  [temp]
  []

[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./coupled_force_u]
    type = CoupledForce
    variable = u
    v = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 2
  [../]
[]

[Functions]
  [err]
    type = ParsedFunction
    value = t
    vars = 'u v'
    vals = '0 1'
  []

  [func]
    type = ParsedFunction
    value = t
    vars = 'u v err temp'
    vals = '0 1 err temp'
  []
[]

[Postprocessors]
  [temp]
    type = ElementAverageValue
    variable = temp
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-10
  l_tol = 1e-12
  nl_max_its = 10
[]

[Outputs]
  file_base = out
  exodus = true
[]
