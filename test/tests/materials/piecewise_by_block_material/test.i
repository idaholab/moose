[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
[]

[AuxVariables]
  [u]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [to_var]
    type = ADMaterialRealAux
    variable = 'u'
    property = coeff
  []
[]

[Materials]
  [coeff_mat]
    type = ADPiecewiseConstantByBlockMaterial
    prop_name = 'coeff'
    subdomain_to_prop_value = '0 4
                               1 2'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
