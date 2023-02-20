[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 20
    dim = 1
  []
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = 2
  []
  [rxn]
    type = RenamedKernel
    variable = u
    base_coeff = 2
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
[]

[Outputs]
  csv = true
[]
