# A Dirac kernel reads a property produced by a material that reads a property from another
# material. Both producers must be computed at the point source.
[Mesh]
  type = GeneratedMesh
  dim = 2
  # odd number of elements so the point does not fall on an element boundary
  nx = 5
  ny = 5
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

[DiracKernels]
  [source]
    type = MaterialPointSource
    point = '0.5 0.5 0'
    variable = u
    material_prop = source_strength
  []
[]

[BCs]
  [all]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Materials]
  # source_strength = 1 + A = 3, only consumed through the Dirac kernel, and A only through
  # source_strength
  [A]
    type = MaterialReinitTest
    property = A
    value = 2
  []
  [source_strength]
    type = MaterialReinitTest
    property = source_strength
    value = 1
    coupled_property = A
  []
[]

[Postprocessors]
  [u_center]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
