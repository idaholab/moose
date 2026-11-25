[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Problem]
  solve = false
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

[Postprocessors]
  [num_dofs]
    type = NumDOFs
  []
[]

[Executors]
  [steady]
    type = SteadyExecutor
    inner_executors = 'first second'
  []
  [first]
    type = BinaryTestExecutor
  []
  [second]
    type = BinaryTestExecutor
  []
[]

[Outputs]
  csv = true
[]
