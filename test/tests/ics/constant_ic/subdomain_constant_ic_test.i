[Mesh]
  file = sq-2blk.e
  uniform_refine = 1
  # This input file uses CONSTANT MONOMIAL AuxVariables, which don't
  # currently work right with ParallelMesh in parallel.  See #2122 for
  # more information.
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
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./ic_u_1]
    type = ConstantIC
    variable = u
    value = 42
    block = '1 2'
  [../]

  [./ic_u_aux_1]
    type = ConstantIC
    variable = u_aux
    value = 6.25
    block = '1'
  [../]
  [./ic_u_aux_2]
    type = ConstantIC
    variable = u_aux
    value = 9.99
    block = '2'
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

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
  nl_rel_tol = 1e-10
[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


