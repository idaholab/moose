[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  allow_renumbering = false
[]

[Variables]
  [T]
  []
[]

[Functions]
  [k_fn]
    type = ParsedFunction
    expression = 't*t + 2*t'
  []

  [cp_fn]
    type = ParsedFunction
    expression = 't*t*t + 3*t'
  []

  [rho_fn]
    type = ParsedFunction
    expression = 't*t*t*t + 4*t'
  []
[]

[HeatStructureMaterials]
  [prop_uo]
    type = SolidMaterialProperties
    k = k_fn
    cp = cp_fn
    rho = rho_fn
  []
[]

[Components]
[]

[Materials]
  [solid_mat]
    type = ADSolidMaterial
    T = T
    properties = prop_uo
  []
[]

[Kernels]
  [td]
    type = ADHeatConductionTimeDerivative
    variable = T
    specific_heat = specific_heat
    density_name = density
  []
  [diff]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = thermal_conductivity
  []
  [forcing_fn]
    type = BodyForce
    variable = T
    value = -4
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    variable = T
    value = 0
  []
  [right]
    type = DirichletBC
    boundary = right
    variable = T
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
[]
