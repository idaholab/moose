[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmax = 1
  ymax = 1
  zmax = 1
[]

[Materials]
  [elasticity]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '1.1e6 0.75e6 0.75e6 1.5e6 0.75e6 1.5e6 0.375e6 0.375e6 0.375e6'
  []
  [elastic_constants]
    type = ConstantsFromElasticityTensor
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
