[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Materials]
  [D_h]
    type = GenericConstantMaterial
    prop_names = 'D_h'
    prop_values = '0.15'
  []

  [props]
    type = ADGenericConstantMaterial
    prop_names = 'rho vel mu '
    prop_values = '1000 0.1 0.001'
  []

  [fD_material]
    type = ADWallFrictionChurchillMaterial
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
    f_D = 'f_D'
    roughness = 0.5
  []
[]

[Postprocessors]
  [fD]
    type = ADElementAverageMaterialProperty
    mat_prop = f_D
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
