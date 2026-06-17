[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
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

[Problem]
  type = GetterTooEarlyProblem
  getter = distribution
  distribution = dist
  sampler = sample
  fv_interpolation_method = interp
[]

[Distributions]
  [dist]
    type = TestDistribution
  []
[]

[Samplers]
  [sample]
    type = TestSampler
  []
[]

[FVInterpolationMethods]
  [interp]
    type = FVGeometricAverage
  []
[]

[Executioner]
  type = Steady
[]
