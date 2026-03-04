[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmax = 1
  ymax = 1
[]

[AuxVariables]
#  [./d2C3313_aux]
#    order = CONSTANT
#    family = MONOMIAL
#  [../]
[]


[AuxKernels]
#  [./matl_d2C3313]
#    type = RankFourAux
#    rank_four_tensor = d^2elasticity_tensor/dc^2
#    index_i = 2
#    index_j = 2
#    index_k = 0
#    index_l = 2
#    variable = d2C3313_aux
#    execute_on = initial
#  [../]
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    block = 0
    youngs_modulus = 200e9
    poissons_ratio = 0.3
  []
  [elastic_constants]
    type = ConstantsFromElasticityTensor
    block = 0
    outputs = all
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
