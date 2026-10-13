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
    type = KokkosDiffusion
    variable = u
  []
  [dt_u]
    type = KokkosTimeDerivative
    variable = u
  []

  [diff_v]
    type = KokkosDiffusion
    variable = v
  []
  [dt_v]
    type = KokkosTimeDerivative
    variable = v
  []
[]

[BCs]
  [flux_u]
    type = KokkosDirectionalFluxBC
    variable = u
    boundary = 0
  []

  [flux_v]
    type = KokkosDirectionalFluxBC
    variable = v
    boundary = 0
    self_shadow_uo = shadow
  []
[]

[Postprocessors]
  [ave_v_all]
    type = KokkosSideAverageValue
    variable = v
    boundary = 0
  []
  [ave_v_exposed]
    type = KokkosExposedSideAverageValue
    variable = v
    boundary = 0
    self_shadow_uo = shadow
  []
[]

[UserObjects]
  [shadow]
    type = KokkosSelfShadowSideUserObject
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
