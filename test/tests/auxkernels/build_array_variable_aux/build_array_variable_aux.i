[Mesh]
  [meshgen]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [a]
    order = FIRST
    family = LAGRANGE
  []
  [b]
    order = FIRST
    family = LAGRANGE
  []
  [c]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [d]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[Kernels]
  [diff_a]
    type = Diffusion
    variable = a
  []
  [diff_b]
    type = Diffusion
    variable = b
  []
[]

[FVKernels]
  [diff_c]
    type = FVDiffusion
    variable = c
    coeff = 1
  []
  [diff_d]
    type = FVDiffusion
    variable = d
    coeff = 1
  []
[]

[BCs]
  [a1]
    type = DirichletBC
    variable = a
    boundary = left
    value = 0
  []
  [a2]
    type = DirichletBC
    variable = a
    boundary = right
    value = 1
  []
  [b1]
    type = DirichletBC
    variable = b
    boundary = bottom
    value = 0
  []
  [b2]
    type = DirichletBC
    variable = b
    boundary = top
    value = 1
  []
[]

[FVBCs]
  [c1]
    type = FVDirichletBC
    variable = c
    boundary = left
    value = 0
  []
  [c2]
    type = FVDirichletBC
    variable = c
    boundary = right
    value = 1
  []
  [d1]
    type = FVDirichletBC
    variable = d
    boundary = bottom
    value = 0
  []
  [d2]
    type = FVDirichletBC
    variable = d
    boundary = top
    value = 1
  []
[]

[Problem]
  kernel_coverage_check = off
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[AuxVariables]
  [ab]
    order = FIRST
    family = LAGRANGE
    components = 2
  []
  [cd]
    order = CONSTANT
    family = MONOMIAL
    components = 2
  []
[]

[AuxKernels]
  [build_ab]
    type = BuildArrayVariableAux
    variable = ab
    component_variables = 'a b'
  []
  [build_cd]
    type = BuildArrayVariableAux
    variable = cd
    component_variables = 'c d'
  []
[]

[Outputs]
  exodus = true
[]
