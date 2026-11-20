# A boundary-restricted stateful material with compute = false (a discrete material) is never
# computed, so its properties keep the values set by initQpStatefulProperties(). The side
# average of the property therefore equals initial_diffusivity only if the discrete material
# itself was used to initialize the stateful data on the boundary.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
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

[Materials]
  [discrete]
    type = SpatialStatefulMaterial
    initial_diffusivity = 5
    boundary = right
    compute = false
  []
[]

[Postprocessors]
  [right_diffusivity]
    type = SideAverageMaterialProperty
    property = diffusivity
    boundary = right
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
