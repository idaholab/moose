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
    prop_names = 'cp mu k'
    prop_values = '1 2 4'
  []

  [Pr_material]
    type = ADPrandtlNumberMaterial
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [Pr]
    type = ADElementAverageMaterialProperty
    mat_prop = Pr
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
