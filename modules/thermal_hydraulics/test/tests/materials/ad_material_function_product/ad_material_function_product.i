# Gold value should be the following:
#   product = scale * func
#           = 0.5 * 100
#           = 50

[GlobalParams]
  execute_on = 'initial'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [func]
    type = ConstantFunction
    value = 100
  []
[]

[Materials]
  [scale_mat]
    type = ADGenericConstantMaterial
    prop_names = 'scale'
    prop_values = '0.5'
  []
  [product_mat]
    type = ADMaterialFunctionProductMaterial
    mat_prop_product = product
    mat_prop_scale = scale
    function = func
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [product_pp]
    type = ADElementAverageMaterialProperty
    mat_prop = product
  []
[]

[Outputs]
  csv = true
[]
