[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmax = 1
  ymax = 1
[]

[AuxVariables]
  [youngs_modulus_from_tensor]
    order = CONSTANT
    family = MONOMIAL
  []
  [poissons_ratio_from_tensor]
    order = CONSTANT
    family = MONOMIAL
  []
  [shear_modulus_from_tensor]
    order = CONSTANT
    family = MONOMIAL
  []
  [bulk_modulus_from_tensor]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [youngs_modulus]
    type = MaterialRealAux
    property = youngs_modulus_from_tensor
    variable = youngs_modulus_from_tensor
    execute_on = initial
  []
  [poissons_ratio]
    type = MaterialRealAux
    property = poissons_ratio_from_tensor
    variable = poissons_ratio_from_tensor
    execute_on = initial
  []
  [shear_modulus]
    type = MaterialRealAux
    property = shear_modulus_from_tensor
    variable = shear_modulus_from_tensor
    execute_on = initial
  []
  [bulk_modulus]
    type = MaterialRealAux
    property = bulk_modulus_from_tensor
    variable = bulk_modulus_from_tensor
    execute_on = initial
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200e9
    poissons_ratio = 0.3
  []
  [elastic_constants]
    type = ConstantsFromElasticityTensor
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [youngs_modulus]
    type = ElementAverageValue
    variable = youngs_modulus_from_tensor
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [poissons_ratio]
    type = ElementAverageValue
    variable = poissons_ratio_from_tensor
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [shear_modulus]
    type = ElementAverageValue
    variable = shear_modulus_from_tensor
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [bulk_modulus]
    type = ElementAverageValue
    variable = bulk_modulus_from_tensor
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
