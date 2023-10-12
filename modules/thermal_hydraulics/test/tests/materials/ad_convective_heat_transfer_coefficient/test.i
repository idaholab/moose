[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Problem]
  solve = false
[]

[Materials]
  [props]
    type = ADGenericConstantMaterial
    prop_names = 'Nu k D_h'
    prop_values = '1000 2 20'
  []

  [Hw_material]
    type = ADConvectiveHeatTransferCoefficientMaterial
    Nu = Nu
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [Hw]
    type = ADElementAverageMaterialProperty
    mat_prop = Hw
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
