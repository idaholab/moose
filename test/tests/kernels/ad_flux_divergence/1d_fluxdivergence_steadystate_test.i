[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [flux]
    type = ADFluxDivergence
    variable = u
  []
[]

[Materials]
  [diffusivity]
    type = ADGenericConstantMaterial
    prop_names = 'diffusivity'
    prop_values = '1.0'
  []

  [flux]
    type = ADFluxFromGradientMaterial
    flux = flux
    u = u
    diffusivity = diffusivity
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

[Outputs]
  file_base = steadystate_out
  exodus = true
[]
