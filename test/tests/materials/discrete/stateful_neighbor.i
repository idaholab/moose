# A stateful material with compute = false (a discrete material) is never computed, so its
# properties keep the values set by initQpStatefulProperties(). InsideUserObject reports the
# square root of the sum of (u - u_neighbor)^2 + (D_old + D_old_neighbor) / 2 over the internal
# side quadrature points. u is continuous, so the first term vanishes. With 4 internal sides
# and 2 quadrature points per side, the result is sqrt(8 * 5) only if the discrete material
# initialized the stateful data on both the element and the neighbor side.

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
    compute = false
  []
[]

[UserObjects]
  [inside]
    type = InsideUserObject
    variable = u
    diffusivity = diffusivity
    use_old_prop = true
  []
[]

[Postprocessors]
  [inside_value]
    type = InsideValuePPS
    user_object = inside
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
