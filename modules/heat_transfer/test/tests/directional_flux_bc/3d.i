[Mesh]
  [planet]
    type = SphereMeshGenerator
    radius = 1
    nr = 2 # increase for a better visualization
  []

  [moon]
    type = SphereMeshGenerator
    radius = 0.3
    nr = 1 # increase for a better visualization
  []
  [combine]
    type = CombinerGenerator
    inputs = 'planet moon'
    positions = '0 0 0 -1.2 -1 -1'
  []
[]

[GlobalParams]
  illumination_flux = '1 1 1'
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [dt_u]
    type = TimeDerivative
    variable = u
  []

  [diff_v]
    type = Diffusion
    variable = v
  []
  [dt_v]
    type = TimeDerivative
    variable = v
  []
[]

[BCs]
  [flux_u]
    type = DirectionalFluxBC
    variable = u
    boundary = 0
  []

  [flux_v]
    type = DirectionalFluxBC
    variable = v
    boundary = 0
    self_shadow_uo = shadow
  []
[]

[Postprocessors]
  [ave_v_all]
    type = SideAverageValue
    variable = v
    boundary = 0
  []
  [ave_v_exposed]
    type = ExposedSideAverageValue
    variable = v
    boundary = 0
    self_shadow_uo = shadow
  []
[]

[UserObjects]
  [shadow]
    type = SelfShadowSideUserObject
    boundary = 0
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 1
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = FINAL
  []
[]
