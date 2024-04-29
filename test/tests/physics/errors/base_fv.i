[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 2'
    dy = '2 1'
    ix = '2 3'
    iy = '3 2'
  []
[]

[Physics]
  [Diffusion]
    [FiniteVolume]
      [diff]
        source_functor = 2
        transient = true
      []
    []
  []
[]

[Executioner]
  type = Steady
[]
