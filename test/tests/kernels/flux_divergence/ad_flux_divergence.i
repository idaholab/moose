[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
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
  [flux]
    type = ADFluxFromGradientMaterial
    flux = flux
    u = u
    diffusivity = 1.0
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [csv]
    type = CSV
    file_base = ad_flux_divergence_out
  []
[]

