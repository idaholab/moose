[Mesh]
  [planet]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    radii = 1
    num_sectors = 10
    rings = 2
    preserve_volumes = false
  []

  [moon]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    radii = 0.5
    num_sectors = 8
    rings = 2
    preserve_volumes = false
  []
  [combine]
    type = CombinerGenerator
    inputs = 'planet moon'
    positions = '0 0 0 -1.5 -0.5 0'
  []
[]

[GlobalParams]
  illumination_flux = '1 1 0'
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
    boundary = outer
  []

  [flux_v]
    type = KokkosDirectionalFluxBC
    variable = v
    boundary = outer
    self_shadow_uo = shadow
  []
[]

[Postprocessors]
  [ave_v_all]
    type = KokkosSideAverageValue
    variable = v
    boundary = outer
  []
  [ave_v_exposed]
    type = KokkosExposedSideAverageValue
    variable = v
    boundary = outer
    self_shadow_uo = shadow
  []
[]

[UserObjects]
  [shadow]
    type = KokkosSelfShadowSideUserObject
    boundary = outer
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
