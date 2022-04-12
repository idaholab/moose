[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
  second_order = true
[]

[AuxVariables]
  [v]
    order = SECOND
    family = MONOMIAL
  []
[]

[AuxKernels]
  [v]
    type = ParsedAux
    variable = v
    function = '3*x^2*y'
    use_xyzt = true
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  file_base = out
  exodus = true
[]
