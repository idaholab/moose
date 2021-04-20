[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [tt]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 100
  []

  [ten]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 1
  []

  [2k]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 2
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxKernels]
  [all]
    variable = tt
    type = MultipleUpdateElemAux
    vars = 'ten 2k'
  []
[]

[BCs]
  active = 'left right'

  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out_multi_elem_var
  exodus = true
[]
