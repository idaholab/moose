[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 1
  []
[]

[Variables]
  [u]
  []
[]

[Materials]
  [mat1]
    type = Kokkos1DRealProperty
    name = 'prop'
    dims = '1'
    boundary = 'left'
  []
  [mat2]
    type = Kokkos1DRealProperty
    name = 'prop'
    dims = '2'
    boundary = 'left'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
