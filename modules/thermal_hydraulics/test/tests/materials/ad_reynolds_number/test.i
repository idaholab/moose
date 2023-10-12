[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Materials]
  [const_mpropsat]
    type = ADGenericConstantMaterial
    prop_names = 'rho vel D_h mu'
    prop_values = '1000 5 0.002 0.1'
  []

  [Re_material]
    type = ADReynoldsNumberMaterial
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [Re]
    type = ADElementAverageMaterialProperty
    mat_prop = Re
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
