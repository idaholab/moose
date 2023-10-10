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
    type = MooseVariableFVReal
    initial_condition = 0
  []

  [ten]
    type = MooseVariableFVReal
    initial_condition = 1
  []

  [2k]
    type = MooseVariableFVReal
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
    type = MultipleUpdateAux
    u = u
    var1 = ten
    var2 = 2k
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

  [Quadrature]
    order = CONSTANT
  []
[]

[Outputs]
  file_base = out_multi_var_fv
  exodus = true
[]
