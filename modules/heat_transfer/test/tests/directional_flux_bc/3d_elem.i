[Mesh]
  [shade]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 3
    nz = 3
    xmax = 0.2
    ymax = 0.5
    zmax = 0.5
  []

  [screen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 20
    nz = 20
    xmax = 0.05
  []
  [screen_block]
    type = SubdomainIDGenerator
    input = screen
    subdomain_id = 1
  []

  [combine]
    type = CombinerGenerator
    inputs = 'shade screen_block'
    positions = '0 0 0 1 0 0'
  []

  [all_sides]
    type = SideSetsAroundSubdomainGenerator
    block = '0 1'
    new_boundary = 100
    input = combine
  []
  [shaded_side]
    type = SideSetsAroundSubdomainGenerator
    normal = '-1 0 0'
    block = 1
    input = all_sides
    new_boundary = 101
  []
[]

[GlobalParams]
  illumination_flux = '1 0 0'
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
  [dt]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [flux]
    type = DirectionalFluxBC
    variable = u
    boundary = 101
    self_shadow_uo = shadow
  []
[]

[UserObjects]
  [shadow]
    type = SelfShadowSideUserObject
    boundary = 100
    execute_on = INITIAL
  []
[]

[Postprocessors]
  [light]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = 101
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  num_steps = 1
[]

[Outputs]
  csv = true
[]
