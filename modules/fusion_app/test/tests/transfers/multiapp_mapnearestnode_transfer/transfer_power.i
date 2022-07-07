[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    xmax = 0.4
    ny = 4
    ymax = 0.4
  []
[]

[Variables]
  [temp][]
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = temp
    diffusivity = D
    block = 0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temp
    value = 0
    boundary = left
  []
  [right]
    type = DirichletBC
    variable = temp
    value = 1
    boundary = right
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[Materials]
  [mat0]
    type = GenericConstantMaterial
    prop_names = 'D'
    prop_values = '1'
  []
[]

[AuxVariables]
  [from_master]
  []
  [from_master_elemental]
    order = CONSTANT
    family = MONOMIAL
  []
  [to_master]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [aux]
    type = ParsedAux
    variable = to_master
    function = 'temp'
    args = 'temp'
  []
[]

[Problem]
  type = FusionProblem
[]
