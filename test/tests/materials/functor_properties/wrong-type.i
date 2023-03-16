[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff_u]
    type = FunctorMatDiffusion
    variable = u
    diffusivity = 'prop'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [request_ad]
    type = FEFVCouplingMaterial
    retrieved_prop_name = 'prop'
  []
  [declare_regular]
    type = ADGenericFunctorMaterial
    prop_names = 'prop'
    prop_values = '1'
  []
[]


[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
